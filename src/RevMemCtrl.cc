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
const char* OpStr( MemOp op ) {
  // clang-format off
  switch( op ) {
  case MemOp::MemOpREAD:        return "MemOpREAD";
  case MemOp::MemOpWRITE:       return "MemOpWRITE";
  case MemOp::MemOpFLUSH:       return "MemOpFLUSH";
  case MemOp::MemOpREADLOCK:    return "MemOpREADLOCK";
  case MemOp::MemOpWRITEUNLOCK: return "MemOpWRITEUNLOCK";
  case MemOp::MemOpLOADLINK:    return "MemOpLOADLINK";
  case MemOp::MemOpSTORECOND:   return "MemOpSTORECOND";
  case MemOp::MemOpCUSTOM:      return "MemOpCUSTOM";
  case MemOp::MemOpFENCE:       return "MemOpFENCE";
  case MemOp::MemOpINV:         return "MemOpINV";
  default:                      return "unknown";
  }
  // clang-format on
}

// ---------------------------------------------------------------
// RevMemCtrl
// ---------------------------------------------------------------
RevMemCtrl::RevMemCtrl( ComponentId_t id, const Params& params )
  : SubComponent( id ), verbose( params.find<uint32_t>( "verbose" ) ),
    output( new SST::Output( "[RevMemCtrl @t]: ", verbose, 0, SST::Output::STDOUT ) ) {}

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
  memOpMax[MemOp::MemOpPERCYCLE]    = params.find<uint32_t>( "ops_per_cycle", 2 );

  memIface                          = loadUserSubComponent<StandardMem>(
    "memIface",
    ComponentInfo::SHARE_NONE,  //*/ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
    getTimeConverter( ClockFreq ),
    new StandardMem::Handler<RevBasicMemCtrl>( this, &RevBasicMemCtrl::processMemEvent )
  );

  if( !memIface ) {
    output->fatal( CALL_INFO, -1, "Error: memory interface is null\n" );
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

/// RevBasicMemCtrl: send a flush request
bool RevBasicMemCtrl::sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, bool Inv ) {
  return QRequest( MemCtrlStats::FlushPending, MemOp::MemOpFLUSH, Hart, Addr, PAddr, Size, flags, Inv );
}

/// RevBasicMemCtrl: send a read request
bool RevBasicMemCtrl::sendREADRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, MemReq req
) {
  return QRequest( MemCtrlStats::ReadPending, MemOp::MemOpREAD, Hart, Addr, PAddr, Size, flags, target, std::move( req ) );
}

/// RevBasicMemCtrl: send a write request
bool RevBasicMemCtrl::sendWRITERequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer
) {
  return QRequest( MemCtrlStats::WritePending, MemOp::MemOpWRITE, Hart, Addr, PAddr, Size, flags, buffer );
}

bool RevBasicMemCtrl::sendREADLOCKRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, MemReq req
) {
  return QRequest( MemCtrlStats::ReadLockPending, MemOp::MemOpREADLOCK, Hart, Addr, PAddr, Size, flags, target, std::move( req ) );
}

// RevBasicMemCtrl: send a writelock request
bool RevBasicMemCtrl::sendWRITELOCKRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer
) {
  return QRequest( MemCtrlStats::WriteUnlockPending, MemOp::MemOpWRITEUNLOCK, Hart, Addr, PAddr, Size, flags, buffer );
}

// RevBasicMemCtrl: send a loadlink request
bool RevBasicMemCtrl::sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags ) {
  return QRequest( MemCtrlStats::LoadLinkPending, MemOp::MemOpLOADLINK, Hart, Addr, PAddr, Size, flags );
}

// RevBasicMemCtrl: send a storecond request
bool RevBasicMemCtrl::sendSTORECONDRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer
) {
  return QRequest( MemCtrlStats::StoreCondPending, MemOp::MemOpSTORECOND, Hart, Addr, PAddr, Size, flags, buffer );
}

// RevBasicMemCtrl: send an void custom read memory request
bool RevBasicMemCtrl::sendCUSTOMREADRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, uint32_t Opc
) {
  return QRequest( MemCtrlStats::CustomPending, MemOp::MemOpCUSTOM, Hart, Addr, PAddr, Size, flags, target, Opc );
}

// RevBasicMemCtrl: send a custom write request
bool RevBasicMemCtrl::sendCUSTOMWRITERequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer, uint32_t Opc
) {
  return QRequest( MemCtrlStats::CustomPending, MemOp::MemOpCUSTOM, Hart, Addr, PAddr, Size, flags, buffer, Opc );
}

bool RevBasicMemCtrl::sendFENCE( uint32_t Hart ) {
  rqstQ.push( std::make_shared<RevMemOp>( MemOp::MemOpFENCE, Hart ) );
  recordStat( MemCtrlStats::FencePending );
  return true;
}

bool RevBasicMemCtrl::sendAMORequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag Flags, uint8_t* Buffer, void* Target, MemReq Req
) {
  MemCtrlStats stat{};
  // clang-format off
  switch( RevFlagAtomic( Flags ) ) {
    case RevFlag::F_AMOADD:   stat = MemCtrlStats::AMOAddPending ; break;
    case RevFlag::F_AMOXOR:   stat = MemCtrlStats::AMOXorPending ; break;
    case RevFlag::F_AMOAND:   stat = MemCtrlStats::AMOAndPending ; break;
    case RevFlag::F_AMOOR:    stat = MemCtrlStats::AMOOrPending  ; break;
    case RevFlag::F_AMOMIN:   stat = MemCtrlStats::AMOMinPending ; break;
    case RevFlag::F_AMOMAX:   stat = MemCtrlStats::AMOMaxPending ; break;
    case RevFlag::F_AMOMINU:  stat = MemCtrlStats::AMOMinuPending; break;
    case RevFlag::F_AMOMAXU:  stat = MemCtrlStats::AMOMaxuPending; break;
    case RevFlag::F_AMOSWAP:  stat = MemCtrlStats::AMOSwapPending; break;
    default: output->fatal( CALL_INFO, -1, "Unknown atomic operation\n" );
  }
  // clang-format on

  // Create a memory operation for the AMO read
  // Since this is a read-modify-write operation, the first RevMemOp is a MemOp::MemOpREAD.
  return QRequest( stat, MemOp::MemOpREAD, Hart, Addr, PAddr, Size, Flags, Target, Buffer, std::move( Req ) );
}

void RevBasicMemCtrl::processMemEvent( StandardMem::Request* ev ) {
  output->verbose( CALL_INFO, 15, 0, "Received memory request event\n" );
  if( ev == nullptr )
    output->fatal( CALL_INFO, -1, "Error: Received null memory event\n" );
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

// --------------------------------------------------------------------
// Cache Handler Logic
// --------------------------------------------------------------------
// Send a memory request based on the RevMemOp and the cache parameters
// Both cached and uncached memory is handled, with one basic loop for
// splitting requests across cache lines if caching is enabled.
// If caching is disabled, a single request is sent regardless of size.
// --------------------------------------------------------------------
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

  if( NumLines > 1 && RevFlagAtomic( op->getFlags() ) != RevFlag::F_NONE )
    output->fatal(
      CALL_INFO,
      -1,
      "Error: Atomic operation spans multiple cache lines\nAddr = 0x%" PRIx64 "; NumLines = %" PRIu32 "; Size = %" PRIu32 "\n",
      base,
      NumLines,
      bytesLeft
    );

#ifdef _REV_DEBUG_
  std::cout << "Building mem request for Addr = 0x" << std::hex << base << std::dec << "; NumLines = " << NumLines
            << "; Size = " << bytesLeft << std::endl;
#endif

  // first determine if we have enough request slots to service all the cache lines
  // if we don't have enough request slots, then requeue the entire RevMemOp
  auto memOp = op->getOp();

  // Hard error if there can NEVER be enough slots available
  if( NumLines > memOpMax[memOp] )
    output->fatal(
      CALL_INFO,
      -1,
      "Error: Memory request %s requires %" PRIu32
      " cache line slots, but the maximum number of outstanding %s requests is %" PRIu32 "\n",
      OpStr( memOp ),
      NumLines,
      OpStr( memOp ),
      memOpMax[memOp]
    );

  // Return false if not enough slots are available right now
  if( NumLines + memOpNum[memOp] > memOpMax[memOp] )
    return false;

#ifdef _REV_DEBUG_
  std::cout << "Found sufficient request slots for " << NumLines << " cache lines" << std::endl;
#endif

  auto flags   = safe_static_cast<flags_t>( hasCache ? op->getStdFlags() : op->getNonCacheFlags() );
  auto curByte = op->getBuf();

  // number of bytes accessed in first cache line containing base address
  // if the cache is disabled, then the first and only request is the entire number of bytes
  auto size    = isCached ? std::min( bytesLeft, lineSize - uint32_t( base % lineSize ) ) : bytesLeft;

  // dispatch the requests
  // starting with the end of the first cache line, then each cache line afterwards
  // this prevents us from sending requests that span multiple cache lines
  while( size ) {
    // clang-format off
    switch( memOp ) {
    case MemOp::MemOpREAD:
      sendMemRqst( op, memOp, MemCtrlStats::ReadInFlight,        new StandardMem::Read( base, size, flags ) );
      break;
    case MemOp::MemOpWRITE:
      sendMemRqst( op, memOp, MemCtrlStats::WriteInFlight,       new StandardMem::Write( base, size, { curByte, curByte + size }, false, flags ) );
      break;
    case MemOp::MemOpFLUSH:
      sendMemRqst( op, memOp, MemCtrlStats::FlushInFlight,       new StandardMem::FlushAddr( base, size, op->getInv(), size, flags ) );
      break;
    case MemOp::MemOpREADLOCK:
      sendMemRqst( op, memOp, MemCtrlStats::ReadLockInFlight,    new StandardMem::ReadLock( base, size, flags ) );
      break;
    case MemOp::MemOpWRITEUNLOCK:
      sendMemRqst( op, memOp, MemCtrlStats::WriteUnlockInFlight, new StandardMem::WriteUnlock( base, size, { curByte, curByte + size }, false, flags ) );
      break;
    case MemOp::MemOpLOADLINK:
      sendMemRqst( op, memOp, MemCtrlStats::LoadLinkInFlight,    new StandardMem::LoadLink( base, size, flags ) );
      break;
    case MemOp::MemOpSTORECOND:
      sendMemRqst( op, memOp, MemCtrlStats::StoreCondInFlight,   new StandardMem::StoreConditional( base, size, { curByte, curByte + size }, flags ) );
      break;
    case MemOp::MemOpCUSTOM:        // TODO: need more support for custom memory ops
      sendMemRqst( op, memOp, MemCtrlStats::CustomInFlight,      new StandardMem::CustomReq( nullptr, flags ) );
      break;

    // we should never get here with a FENCE operation
    // the FENCE is handled locally and never dispatched on the memIface
    default: output->fatal( CALL_INFO, -1, "Error: unknown memory operation type\n" );
    }
    // clang-format on

    base += size;
    curByte += size;
    bytesLeft -= size;

    // setup the adjusted size of the next request
    size = std::min( bytesLeft, lineSize );
  }

  return true;
}

// Handle flags after a reading "size" bytes, sign- or zero-extending it or NaN-boxing it
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
  }
  // clang-format on
}

// Perform an atomic operation on target data which has been read
// Return the value which should be written back to memory
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

// Handle an atomic operation response after the read request completes
// Immediately send a write request of the modified data
void RevBasicMemCtrl::handleAMOResp( const std::shared_ptr<RevMemOp>& readOp ) {
  auto flags  = readOp->getFlags();
  auto size   = readOp->getSize();

  // Perform the AMO operation on the already-loaded data
  auto newMem = performAMO( flags, size, readOp->getTarget(), readOp->getBuf() );

  // Immediately issue the Write request after the Read request finishes. The rqstQ only
  // waits for the atomic Read to complete before it issues other memory requests. This
  // ensures the Read-modify-Write is atomic w.r.t the issuance of other memory requests.
  sendMemRqst(
    std::make_shared<RevMemOp>( MemOp::MemOpWRITE, size, readOp->getHart(), readOp->getAddr(), readOp->getPhysAddr(), flags ),
    MemOp::MemOpWRITE,
    MemCtrlStats::WriteInFlight,
    new StandardMem::Write( readOp->getAddr(), size, { newMem.uc, newMem.uc + size }, false, safe_static_cast<flags_t>( flags ) )
  );
}

// Send a memory request to the SST interface, tracking its counters and statistics
// A hash table (outstanding) maps the StandardMem::Request ID to the RevMemOp
// A binary tree (hartOutstanding) maps a Hart ID to multiple RevMemOp requests
void RevBasicMemCtrl::sendMemRqst(
  const std::shared_ptr<RevMemOp>& op, MemOp memOp, MemCtrlStats stat, StandardMem::Request* rqst
) {
  ++op->rqstCount();  // Increment the request reference count of the RevMemOp
  ++memOpNum[memOp];  // Increment the number of outstanding requests for this MemOp

  // Map the request ID to an iterator pointing to mapping from hart to RevMemOp
  if( !outstanding.try_emplace( rqst->getID(), hartOutstanding.emplace( op->getHart(), op ) ).second )
    output->fatal( CALL_INFO, -1, "Error: %s memory request with the same ID added twice\n", OpStr( memOp ) );

  recordStat( stat );      // Record the statistic
  memIface->send( rqst );  // Send the request
}

// Handle memory requests when they complete in SST.
template<MemOp memOp, typename RESP>
void RevBasicMemCtrl::handleResp( RESP* ev ) {
  // Extract (remove) the request based on ID
  auto node = outstanding.extract( ev->getID() );
  if( node.empty() )
    output->fatal( CALL_INFO, -1, "Internal Error: Outstanding memory request not found in %s handle\n", OpStr( memOp ) );

  // An iterator to the Hart --> RevMemOp mapping entry
  auto hartOutstandingEntry           = node.mapped();

  // A shared_ptr to the RevMemOp
  const std::shared_ptr<RevMemOp>& op = hartOutstandingEntry->second;

  // For read requests, copy the read data to the portion of the Rev memory target
  if constexpr( memOp == MemOp::MemOpREAD ) {
    memcpy( static_cast<uint8_t*>( op->getTarget() ) + ( ev->pAddr - op->getAddr() ), &ev->data[0], ev->size );
  }

  // Decrement the number of outstanding requests for this MemOp
  if( !memOpNum[memOp]-- )
    output->fatal(
      CALL_INFO, -1, "Internal Error: Outstanding %s request count is zero during response handler\n", OpStr( memOp )
    );

  // Decrement the RevMemOp's request reference count
  // Complete the RevMemOp if there are no more requests associated with this RevMemOp
  if( !--op->rqstCount() ) {
    if constexpr( memOp == MemOp::MemOpREAD ) {  // handleReadResp
      if( RevFlagAtomic( op->getFlags() ) != RevFlag::F_NONE )
        handleAMOResp( op );  // perform an atomic operation and send a WRITE request

      // Determine if we need to sign/zero extend or NaN-box the destination register
      RevHandleFlagResp( op->getTarget(), op->getSize(), op->getFlags() );

      // Mark the load (read) as complete, even if the write of an atomic operation has
      // not completed, because the read has completed, the destination register has been
      // modified, and the write with the new data has been sent. Other memory operations
      // on the same hart will wait for the write to finish only if the atomic operation
      // has Acquire semantics or if the following memory operation has Release semantics.
      op->getMemReq().MarkLoadComplete();
    }
  }

  // Delete the entry mapping the hart to this request (invalidates op)
  hartOutstanding.erase( hartOutstandingEntry );

  // Delete the StandardMem request
  delete ev;
}

template void RevBasicMemCtrl::handleResp<MemOp::MemOpREAD>( StandardMem::ReadResp* );
template void RevBasicMemCtrl::handleResp<MemOp::MemOpWRITE>( StandardMem::WriteResp* );
template void RevBasicMemCtrl::handleResp<MemOp::MemOpFLUSH>( StandardMem::FlushResp* );
template void RevBasicMemCtrl::handleResp<MemOp::MemOpCUSTOM>( StandardMem::CustomResp* );
template void RevBasicMemCtrl::handleResp<MemOp::MemOpINV>( StandardMem::InvNotify* );

// Determine whether a memory operation should be stalled based on its flags and the
// state of outstanding memory operations on the same hart
bool RevBasicMemCtrl::isPendingAMO( const std::shared_ptr<RevMemOp>& thisOp ) {
  // [it,end) is the range of outstanding memory requests on the same hart
  auto [it, end] = hartOutstanding.equal_range( thisOp->getHart() );

  // If there are no outstanding memory operations in the same hart, do not stall
  if( it == end )
    return false;

  // If this is a release, stall if there are any outstanding requests in the same hart
  if( RevFlagHas( thisOp->getFlags(), RevFlag::F_RL ) )
    return true;

  // Go through all outstanding memory operations for this hart
  do {
    const auto& op    = it->second;
    RevFlag     flags = op->getFlags();

    // If any outstanding request in the same hart has the Acquire flag, delay this request
    if( RevFlagHas( flags, RevFlag::F_AQ ) )
      return true;

    // If any outstanding request in the same hart is an atomic read, delay this request
    // until the atomic read is completed and the corresponding atomic write is issued
    if( RevFlagAtomic( flags ) != RevFlag::F_NONE && op->getOp() == MemOp::MemOpREAD )
      return true;

  } while( ++it != end );

  return false;
}

// Process the next request in the queue, returning true if further requests should be
// considered in the same clock. If there a fence operation in progress, do not submit
// any memory requests during the current clock as long as any requests are outstanding.
// TODO: rqstQ should be made per-hart as an iterator to a multimap<hart, RevMemOp>
bool RevBasicMemCtrl::processNextRqst() {
  // If there are no queued requests, stop processing this cycle
  if( rqstQ.empty() )
    return false;

  // Get the request at the front of the queue
  const std::shared_ptr<RevMemOp>& op   = rqstQ.front();
  uint32_t                         hart = op->getHart();

  // If the front request is a memory fence, set the hart memory fence flag and continue
  if( op->getOp() == MemOp::MemOpFENCE ) {
    if( hartOutstanding.find( hart ) != hartOutstanding.end() ) {
      // wait for the outstanding ops to clear before processing any more memory requests
      recordStat( MemCtrlStats::FencePending );
      return false;
    } else {
      rqstQ.pop();
      return true;
    }
  }

  // Determine if any Acquire/Release flags or atomic operations would prevent
  // us from dispatching this request.
  if( isPendingAMO( op ) )
    return false;

  // Try sending a StandardMem request; if successful, remove it and continue
  if( buildStandardMemRqst( op ) ) {
    rqstQ.pop();
    return true;
  }

  // If we reach this point, then no requests can be dispatched at this time.
  return false;
}

// For a clock cycle, process queued memory requests until they block or the
// maximum number of requests per cycle is reached.
bool RevBasicMemCtrl::clockTick( Cycle_t cycle ) {
  // Dequeue requests until they block or the maximum number of requests per cycle is reached
  memOpNum[MemOp::MemOpPERCYCLE] = 0;
  while( processNextRqst() && ++memOpNum[MemOp::MemOpPERCYCLE] < memOpMax[MemOp::MemOpPERCYCLE] )
    ;
  return false;
}

}  // namespace SST::RevCPU

// EOF
