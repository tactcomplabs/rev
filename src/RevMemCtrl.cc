//
// _RevMemCtrl_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevMemCtrl.h"
#include "RevRegFile.h"

namespace SST::RevCPU {

/// MemOp: Formatted Output
std::ostream& operator<<( std::ostream& os, MemOp op ) {
  // clang-format off
  switch(op){
    case MemOp::MemOpREAD:        return os << "MemOpREAD";
    case MemOp::MemOpWRITE:       return os << "MemOpWRITE";
    case MemOp::MemOpFLUSH:       return os << "MemOpFLUSH";
    case MemOp::MemOpREADLOCK:    return os << "MemOpREADLOCK";
    case MemOp::MemOpWRITEUNLOCK: return os << "MemOpWRITEUNLOCK";
    case MemOp::MemOpLOADLINK:    return os << "MemOpLOADLINK";
    case MemOp::MemOpSTORECOND:   return os << "MemOpSTORECOND";
    case MemOp::MemOpCUSTOM:      return os << "MemOpCUSTOM";
    case MemOp::MemOpFENCE:       return os << "MemOpFENCE";
    case MemOp::MemOpAMO:         return os << "MemOpAMO";
    default:                      return os;
  }
  // clang-format on
}

// ---------------------------------------------------------------
// RevMemCtrl
// ---------------------------------------------------------------
RevMemCtrl::RevMemCtrl( ComponentId_t id, const Params& params ) : SubComponent( id ) {
  uint32_t verbosity = params.find<uint32_t>( "verbose" );
  output             = std::make_unique<SST::Output>( "[RevMemCtrl @t]: ", verbosity, 0, SST::Output::STDOUT );
}

// ---------------------------------------------------------------
// RevBasicMemCtrl
// ---------------------------------------------------------------
RevBasicMemCtrl::RevBasicMemCtrl( ComponentId_t id, const Params& params ) : RevMemCtrl( id, params ) {
  std::string ClockFreq             = params.find<std::string>( "clock", "1Ghz" );

  memOpMax[MemOp::MemOpREAD]        = params.find<uint32_t>( "max_loads", 64 );
  memOpMax[MemOp::MemOpWRITE]       = params.find<uint32_t>( "max_stores", 64 );
  memOpMax[MemOp::MemOpFLUSH]       = params.find<uint32_t>( "max_flush", 64 );
  memOpMax[MemOp::MemOpLOADLINK]    = params.find<uint32_t>( "max_llsc", 64 );
  memOpMax[MemOp::MemOpREADLOCK]    = params.find<uint32_t>( "max_readlock", 64 );
  memOpMax[MemOp::MemOpWRITEUNLOCK] = params.find<uint32_t>( "max_writeunlock", 64 );
  memOpMax[MemOp::MemOpCUSTOM]      = params.find<uint32_t>( "max_custom", 64 );
  memOpMax[MemOp::MemOpTOTAL]       = params.find<uint32_t>( "ops_per_cycle", 2 );

  memIface                          = loadUserSubComponent<Interfaces::StandardMem>(
    "memIface",
    ComponentInfo::SHARE_NONE,  //*/ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
    getTimeConverter( ClockFreq ),
    new StandardMem::Handler<RevBasicMemCtrl>( this, &RevBasicMemCtrl::processMemEvent )
  );

  if( !memIface ) {
    output->fatal( CALL_INFO, -1, "Error : memory interface is null\n" );
  }

  registerStats();

  registerClock( ClockFreq, new Clock::Handler<RevBasicMemCtrl>( this, &RevBasicMemCtrl::clockTick ) );
}

void RevBasicMemCtrl::registerStats() {
  for( auto* stat : {
         "ReadInFlight",    "ReadPending",         "ReadBytes",          "WriteInFlight",    "WritePending",
         "WriteBytes",      "FlushInFlight",       "FlushPending",       "ReadLockInFlight", "ReadLockPending",
         "ReadLockBytes",   "WriteUnlockInFlight", "WriteUnlockPending", "WriteUnlockBytes", "LoadLinkInFlight",
         "LoadLinkPending", "StoreCondInFlight",   "StoreCondPending",   "CustomInFlight",   "CustomPending",
         "CustomBytes",     "FencePending",        "AMOAddBytes",        "AMOAddPending",    "AMOXorBytes",
         "AMOXorPending",   "AMOAndBytes",         "AMOAndPending",      "AMOOrBytes",       "AMOOrPending",
         "AMOMinBytes",     "AMOMinPending",       "AMOMaxBytes",        "AMOMaxPending",    "AMOMinuBytes",
         "AMOMinuPending",  "AMOMaxuBytes",        "AMOMaxuPending",     "AMOSwapBytes",     "AMOSwapPending",
       } ) {
    stats.push_back( registerStatistic<uint64_t>( stat ) );
  }
}

bool RevBasicMemCtrl::sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, bool Inv, RevFlag flags ) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, MemOp::MemOpFLUSH, flags );
    Op->setInv( Inv );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::FlushPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendREADRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, target, MemOp::MemOpREAD, flags );
    Op->setMemReq( req );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::ReadPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendWRITERequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, buffer, MemOp::MemOpWRITE, flags );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::WritePending );
  }
  return true;
}

bool RevBasicMemCtrl::sendAMORequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, void* target, const MemReq& req, RevFlag flags
) {
  // Check to see if our flags contain an atomic request
  if( RevFlagAtomic( flags ) == RevFlag::F_NONE ) {
    // not an atomic request
    return true;
  }

  // Create a memory operation for the AMO read
  // Since this is a read-modify-write operation, the first RevMemOp is a MemOp::MemOpREAD.
  auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, buffer, target, MemOp::MemOpREAD, flags );
  Op->setMemReq( req );
  rqstQ.emplace_back( std::move( Op ) );

  // now we record the stat for the particular AMO
  // clang-format off
  switch( RevFlagAtomic( flags ) ) {
    case RevFlag::F_AMOADD:   recordStat( MemCtrlStats::AMOAddPending   ); break;
    case RevFlag::F_AMOXOR:   recordStat( MemCtrlStats::AMOXorPending   ); break;
    case RevFlag::F_AMOAND:   recordStat( MemCtrlStats::AMOAndPending   ); break;
    case RevFlag::F_AMOOR:    recordStat( MemCtrlStats::AMOOrPending    ); break;
    case RevFlag::F_AMOMIN:   recordStat( MemCtrlStats::AMOMinPending   ); break;
    case RevFlag::F_AMOMAX:   recordStat( MemCtrlStats::AMOMaxPending   ); break;
    case RevFlag::F_AMOMINU:  recordStat( MemCtrlStats::AMOMinuPending  ); break;
    case RevFlag::F_AMOMAXU:  recordStat( MemCtrlStats::AMOMaxuPending  ); break;
    case RevFlag::F_AMOSWAP:  recordStat( MemCtrlStats::AMOSwapPending  ); break;
    default: break;
  };
  // clang-format on
  return true;
}

bool RevBasicMemCtrl::sendREADLOCKRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, target, MemOp::MemOpREADLOCK, flags );
    Op->setMemReq( req );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::ReadLockPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendWRITELOCKRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, buffer, MemOp::MemOpWRITEUNLOCK, flags );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::WriteUnlockPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags ) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, MemOp::MemOpLOADLINK, flags );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::LoadLinkPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendSTORECONDRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, buffer, MemOp::MemOpSTORECOND, flags );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::StoreCondPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMREADRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, uint32_t Opc, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, target, Opc, MemOp::MemOpCUSTOM, flags );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::CustomPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMWRITERequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, uint32_t Opc, RevFlag flags
) {
  if( Size ) {
    auto Op = std::make_shared<RevMemOp>( Hart, Addr, PAddr, Size, buffer, Opc, MemOp::MemOpCUSTOM, flags );
    rqstQ.emplace_back( std::move( Op ) );
    recordStat( MemCtrlStats::CustomPending );
  }
  return true;
}

bool RevBasicMemCtrl::sendFENCE( uint32_t Hart ) {
  auto Op = std::make_shared<RevMemOp>( Hart, 0, 0, 0, MemOp::MemOpFENCE, RevFlag::F_NONE );
  rqstQ.emplace_back( std::move( Op ) );
  recordStat( MemCtrlStats::FencePending );
  return true;
}

void RevBasicMemCtrl::processMemEvent( StandardMem::Request* ev ) {
  output->verbose( CALL_INFO, 15, 0, "Received memory request event\n" );
  if( ev == nullptr )
    output->fatal( CALL_INFO, -1, "Error : Received null memory event\n" );
  ev->handle( stdMemHandlers.get() );
}

void RevBasicMemCtrl::init( uint32_t phase ) {
  memIface->init( phase );

  // query the caching infrastructure
  if( phase == 1 ) {
    lineSize = uint32_t( memIface->getLineSize() );
    hasCache = lineSize > 0;
    if( hasCache ) {
      output->verbose( CALL_INFO, 5, 0, "Detected cache layers; default line size=%" PRIu32 "\n", lineSize );
    } else {
      output->verbose( CALL_INFO, 5, 0, "No cache detected; disabling caching\n" );
    }
  }
}

/// RevBasicMemCtrl: Add a new memory request
void RevBasicMemCtrl::addMemRqst( const std::shared_ptr<RevMemOp>& op, Interfaces::StandardMem::Request* rqst ) {
  // Map the request ID to a RevMemOp shared_ptr and iterator pointing to mapping from hart to op
  if( !outstanding.try_emplace( rqst->getID(), op, hartOutstanding.emplace( op->getHart(), op ) ).second )
    output->fatal( CALL_INFO, -1, "Error: Memory request with the same ID added twice\n" );

  // Increment the request count of the RevMemOp
  ++op->rqstCount();

  // Send the request
  memIface->send( rqst );
}

// -------------------------------------------------------------
// Cache Handler Logic
// -------------------------------------------------------------
bool RevBasicMemCtrl::buildStandardMemRqst( const std::shared_ptr<RevMemOp>& op ) {
#ifdef _REV_DEBUG_
  std::cout << "building mem request for addr=0x" << std::hex << op->getAddr() << std::dec << "; flags = 0x" << std::hex
            << flags_t( op->getFlags() ) << std::dec << std::endl;
  if( !lineSize )
    std::cout << "WARNING: lineSize == 0!" << std::endl;
  else if( op->getAddr() % lineSize )
    std::cout << "WARNING: address is not cache aligned!" << std::endl;
  if( !isCacheable( op->getFlags() ) )
    std::cout << "WARNING: operation is not cache-able!" << std::endl;
#endif

  uint32_t bytesLeft = op->getSize();
  uint64_t base      = op->getAddr();
  bool     isCached  = hasCache && isCacheable( op->getFlags() );  // cache is enabled and we want to cache the request

  // The size of the segment plus the address offset within the line takes a certain number of lines
  // if the cache is disabled, then 1; eg, there is a 1-to-1 mapping of CPU memops to memory requests
  uint32_t NumLines  = isCached ? uint32_t( ( base % lineSize + bytesLeft - 1 ) / lineSize + 1 ) : 1;

#ifdef _REV_DEBUG_
  std::cout << "Building mem request for addr=0x" << std::hex << base << std::dec << "; NumLines = " << NumLines
            << "; Size = " << bytesLeft << std::endl;
#endif

  // first determine if we have enough request slots to service all the cache lines
  // if we don't have enough request slots, then requeue the entire RevMemOp
  auto memOp = op->getOp();
  if( NumLines + memOpNum[memOp] > memOpMax[memOp] )
    return false;

#ifdef _REV_DEBUG_
  std::cout << "Found sufficient request slots for " << NumLines << " cache lines" << std::endl;
#endif

  auto flags   = safe_static_cast<flags_t>( hasCache ? op->getStdFlags() : op->getNonCacheFlags() );
  auto curByte = op->getBuf().begin();

  // number of bytes accessed in first cache line containing base address
  // if the cache is disabled, then the first and only request is the entire number of bytes
  auto size    = isCached ? std::min( bytesLeft, lineSize - uint32_t( base % lineSize ) ) : bytesLeft;

  // dispatch the requests
  // starting with the end of the first cache line, then each cache line afterwards
  // this prevents us from sending requests that span multiple cache lines
  while( size ) {
    switch( memOp ) {
    case MemOp::MemOpREAD:
#ifdef _REV_DEBUG_
      std::cout << "<<<< READ REQUEST >>>>" << std::endl;
#endif
      addMemRqst( op, new Interfaces::StandardMem::Read( base, size, flags ) );
      recordStat( MemCtrlStats::ReadInFlight );
      break;

    case MemOp::MemOpWRITE:
#ifdef _REV_DEBUG_
      std::cout << "<<<< WRITE REQUEST >>>>" << std::endl;
#endif
      addMemRqst( op, new Interfaces::StandardMem::Write( base, size, { curByte, curByte + size }, false, flags ) );
      recordStat( MemCtrlStats::WriteInFlight );
      break;

    case MemOp::MemOpFLUSH:
      addMemRqst( op, new Interfaces::StandardMem::FlushAddr( base, size, op->getInv(), size, flags ) );
      recordStat( MemCtrlStats::FlushInFlight );
      break;

    case MemOp::MemOpREADLOCK:
      addMemRqst( op, new Interfaces::StandardMem::ReadLock( base, size, flags ) );
      recordStat( MemCtrlStats::ReadLockInFlight );
      break;

    case MemOp::MemOpWRITEUNLOCK:
      addMemRqst( op, new Interfaces::StandardMem::WriteUnlock( base, size, { curByte, curByte + size }, false, flags ) );
      recordStat( MemCtrlStats::WriteUnlockInFlight );
      break;

    case MemOp::MemOpLOADLINK:
      addMemRqst( op, new Interfaces::StandardMem::LoadLink( base, size, flags ) );
      recordStat( MemCtrlStats::LoadLinkInFlight );
      break;

    case MemOp::MemOpSTORECOND:
      addMemRqst( op, new Interfaces::StandardMem::StoreConditional( base, size, { curByte, curByte + size }, flags ) );
      recordStat( MemCtrlStats::StoreCondInFlight );
      break;

    case MemOp::MemOpCUSTOM:
      // TODO: need more support for custom memory ops
      addMemRqst( op, new Interfaces::StandardMem::CustomReq( nullptr, flags ) );
      recordStat( MemCtrlStats::CustomInFlight );
      break;

      // we should never get here with a FENCE operation
      // the FENCE is handled locally and never dispatched on the memIface
    case MemOp::MemOpFENCE:
    default: output->fatal( CALL_INFO, -1, "Error : unknown memory operation type\n" );
    }

    ++memOpNum[memOp];
    base += size;
    curByte += size;
    bytesLeft -= size;

    // setup the adjusted size of the next request
    size = std::min( bytesLeft, lineSize );
  }

  return true;
}

void RevBasicMemCtrl::RevHandleFlagResp( void* target, size_t size, RevFlag flags ) {
  if( RevFlagHas( flags, RevFlag::F_BOXNAN ) && size < sizeof( double ) ) {
    BoxNaN( static_cast<double*>( target ), static_cast<float*>( target ) );
  } else {
    switch( size ) {
    case 1:
      if( RevFlagHas( flags, RevFlag::F_SEXT32 ) ) {
        RevConvertInt<int8_t, int32_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_ZEXT32 ) ) {
        RevConvertInt<uint8_t, uint32_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_SEXT64 ) ) {
        RevConvertInt<int8_t, int64_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_ZEXT64 ) ) {
        RevConvertInt<uint8_t, uint64_t>( target );
      }
      break;
    case 2:
      if( RevFlagHas( flags, RevFlag::F_SEXT32 ) ) {
        RevConvertInt<int16_t, int32_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_ZEXT32 ) ) {
        RevConvertInt<uint16_t, uint32_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_SEXT64 ) ) {
        RevConvertInt<int16_t, int64_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_ZEXT64 ) ) {
        RevConvertInt<uint16_t, uint64_t>( target );
      }
      break;
    case 4:
      if( RevFlagHas( flags, RevFlag::F_SEXT64 ) ) {
        RevConvertInt<int32_t, int64_t>( target );
      } else if( RevFlagHas( flags, RevFlag::F_ZEXT64 ) ) {
        RevConvertInt<uint32_t, uint64_t>( target );
      }
    }
  }
}

///< Apply Atomic Memory Operation
/// The operation described by "flags" is applied to memory "Target" with value "value"
template<typename T>
static std::enable_if_t<!std::is_floating_point_v<T>> ApplyAMO( RevFlag flags, void* Target, T value ) {
  // Target and value cast to signed and uint32_t versions
  auto* TmpTarget  = static_cast<std::make_signed_t<T>*>( Target );
  auto* TmpTargetU = static_cast<std::make_unsigned_t<T>*>( Target );
  auto  TmpBuf     = static_cast<std::make_signed_t<T>>( value );
  auto  TmpBufU    = static_cast<std::make_unsigned_t<T>>( value );

  // clang-format off
  switch( RevFlagAtomic( flags ) ){
    case RevFlag::F_AMOADD:   *TmpTarget += TmpBuf; break;
    case RevFlag::F_AMOXOR:   *TmpTarget ^= TmpBuf; break;
    case RevFlag::F_AMOAND:   *TmpTarget &= TmpBuf; break;
    case RevFlag::F_AMOOR:    *TmpTarget |= TmpBuf; break;
    case RevFlag::F_AMOSWAP:  *TmpTarget  = TmpBuf; break;
    case RevFlag::F_AMOMIN:   *TmpTarget  = std::min( *TmpTarget,  TmpBuf ); break;
    case RevFlag::F_AMOMAX:   *TmpTarget  = std::max( *TmpTarget,  TmpBuf ); break;
    case RevFlag::F_AMOMINU:  *TmpTargetU = std::min( *TmpTargetU, TmpBufU ); break;
    case RevFlag::F_AMOMAXU:  *TmpTargetU = std::max( *TmpTargetU, TmpBufU ); break;
    default: break;
  };
  // clang-format on
}

AMOData RevBasicMemCtrl::performAMO( RevFlag flags, uint32_t size, void* target, const void* data ) {
  AMOData src, newMem;

  // Copy the rs2 source register value
  memcpy( &src, data, size );

  // Copy the original memory value into New memory
  memcpy( &newMem, target, size );

  // Perform the atomic operation
  switch( size ) {
  case 4: ApplyAMO( flags, &newMem, src.u32 ); break;
  case 8: ApplyAMO( flags, &newMem, src.u64 ); break;
  }

  // Return the new value to be written to memory
  return newMem;
}

void RevBasicMemCtrl::handleAMOResp( const std::shared_ptr<RevMemOp>& readOp ) {
  auto flags  = readOp->getFlags();
  auto size   = readOp->getSize();

  // Perform the AMO operation on the already-loaded data
  auto newMem = performAMO( flags, size, readOp->getTarget(), &readOp->getBuf()[0] );

  // Build the memory request that will write the modified value to memory
  auto writeOp =
    std::make_shared<RevMemOp>( readOp->getHart(), readOp->getAddr(), readOp->getPhysAddr(), size, MemOp::MemOpWRITE, flags );

  // Move the memory request object, but DO NOT mark the load as complete.
  // The actual write response from the read-modify-write process will mark the
  // load as complete. At this point, move the MemReq object to the new request.
  writeOp->setMemReq( std::move( readOp->getMemReq() ) );

  // Immediately issue the Write request after the Read request finishes. The
  // rqstQ only waits for the atomic Read to complete before it issues other
  // memory requests. This ensures the Read-modify-Write is atomic w.r.t the
  // issuance of other memory requests.
  addMemRqst(
    writeOp,
    new Interfaces::StandardMem::Write(
      writeOp->getAddr(), size, { newMem.uc, newMem.uc + size }, false, safe_static_cast<flags_t>( flags )
    )
  );
  ++memOpNum[MemOp::MemOpWRITE];
  recordStat( MemCtrlStats::WriteInFlight );
}

template<typename RESP>
void RevBasicMemCtrl::handleResp( RESP* ev, const char* name ) {
  auto node = outstanding.extract( ev->getID() );
  if( node.empty() )
    output->fatal( CALL_INFO, -1, "Outstanding memory request not found in handle%s\n", name );
  const auto& [op, hartOutstandingEntry] = node.mapped();

  // For read requests, copy the read data to the portion of the Rev memory target
  if constexpr( std::is_same_v<RESP, StandardMem::ReadResp> ) {
    memcpy( static_cast<uint8_t*>( op->getTarget() ) + ( ev->pAddr - op->getAddr() ), &ev->data[0], ev->size );
  }

  delete ev;                                      // delete the StandardMem request
  hartOutstanding.erase( hartOutstandingEntry );  // delete the entry mapping harts to outstanding requests

  if( !--op->rqstCount() ) {  // If there are no more requests associated with this RevMemOp
    // handleReadResp
    if constexpr( std::is_same_v<RESP, StandardMem::ReadResp> ) {
      // determine if we need to sign/zero extend or NaN-box the read value
      RevHandleFlagResp( op->getTarget(), op->getSize(), op->getFlags() );

      // determine if we have an atomic request associated with this read operation
      if( RevFlagAtomic( op->getFlags() ) != RevFlag::F_NONE ) {
        handleAMOResp( op );  // perform the atomic operation and generate a WRITE request
      } else {
        op->getMemReq().MarkLoadComplete();  // for non-atomic reads, mark load complete
      }
    }
    // handleWriteResp
    if constexpr( std::is_same_v<RESP, StandardMem::WriteResp> ) {
      // determine if we have an atomic request associated with this write operation
      if( RevFlagAtomic( op->getFlags() ) != RevFlag::F_NONE ) {
        op->getMemReq().MarkLoadComplete();  // mark the original read complete after write is completed
      }
    }
  }
}

// TODO: handle fence operations here?
bool RevBasicMemCtrl::isPending( const std::shared_ptr<RevMemOp>& thisOp ) {
  bool is_release = RevFlagHas( thisOp->getFlags(), RevFlag::F_RL );

  // Go through all outstanding memory operations for this hart
  for( auto [it, end] = hartOutstanding.equal_range( thisOp->getHart() ); it != end; ++it ) {
    // If this request has the Release flag, delay it if there are any outstanding requests in same hart
    if( is_release )
      return true;

    const auto& op    = it->second;
    RevFlag     flags = op->getFlags();

    // If any outstanding request in the same hart has the Acquire flag, delay this request
    if( RevFlagHas( flags, RevFlag::F_AQ ) )
      return true;

    // If any outstanding request in the same hart is an atomic read, delay this request
    // until the atomic read is completed and the corresponding atomic write is issued
    if( RevFlagAtomic( flags ) != RevFlag::F_NONE && op->getOp() == MemOp::MemOpREAD )
      return true;
  }
  return false;
}

bool RevBasicMemCtrl::processNextRqst( MemOpParams& memOps ) {
  // retrieve the next candidate memory operation
  for( auto Slot = rqstQ.cbegin(); Slot != rqstQ.cend(); ++Slot ) {
    MemOp memOp = ( *Slot )->getOp();

    if( memOp == MemOp::MemOpFENCE ) {
      // time to fence!
      // saturate and exit this cycle
      // no need to build a StandardMem request
      rqstQ.erase( Slot );
      ++memOpNum[MemOp::MemOpFENCE];
      return false;
    }

    // Determine if we have any Acquire/Release flags or atomic operations
    // that would prevent us from dispatching this request.
    // Note that we do this AFTER processing FENCE requests.
    if( isPending( *Slot ) )
      return false;

    // If there are request slots available for this operation
    if( memOps[memOp] < memOpMax[memOp] ) {
      // build a StandardMem request
      if( buildStandardMemRqst( *Slot ) ) {
        rqstQ.erase( Slot );          // Sent the request; remove it
        ++memOps[memOp];              // Increment the number of this kind of memory request for this clock
        ++memOps[MemOp::MemOpTOTAL];  // Increment the total number of memory requests for this clock
        return true;
      } else {
        // stop processing any more memory requests for this clock
        // otherwise, this request will induce an infinite loop
        // since we leave the current (failed) request in the queue
        return false;
      }
    }
  }

  // if we reach this point, then we've attempted to
  // process all the potential requests.  none exist
  // that can be dispatched at this time.
#ifdef _REV_DEBUG_
  uint32_t i = 0;
  for( auto it = rqstQ.begin(); it != rqstQ.end(); ++it, ++i ) {
    std::cout << "rqstQ[" << i << "] = " << ( *it )->getOp() << " @ 0x" << std::hex << ( *it )->getAddr() << std::dec
              << "; physAddr = 0x" << std::hex << ( *it )->getPhysAddr() << std::dec << std::endl;
  }
#endif

  return false;
}

bool RevBasicMemCtrl::clockTick( Cycle_t cycle ) {
  // check to see if the top request is a FENCE
  if( memOpNum[MemOp::MemOpFENCE] > 0 ) {
    if( getTotalRqsts() ) {
      // waiting for the outstanding ops to clear
      recordStat( MemCtrlStats::FencePending );
      return false;
    } else {
      // clear the fence and continue processing
      --memOpNum[MemOp::MemOpFENCE];
    }
  }

  // process the memory queue
  MemOpParams memOps;
  while( processNextRqst( memOps ) && memOps[MemOp::MemOpTOTAL] < memOpMax[MemOp::MemOpTOTAL] )
    ;
  return false;
}

}  // namespace SST::RevCPU

// EOF
