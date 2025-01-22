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
  }
  // clang-format on
  return os;
}

// ---------------------------------------------------------------
// RevMemOp
// ---------------------------------------------------------------
RevMemOp::RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, MemOp Op, RevFlag flags )
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( 0 ), SplitRqst( 1 ),
    flags( flags ), target( nullptr ), procReq() {}

RevMemOp::RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, MemOp Op, RevFlag flags )
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( 0 ), SplitRqst( 1 ),
    flags( flags ), target( target ), procReq() {}

RevMemOp::RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, MemOp Op, RevFlag flags )
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( 0 ), SplitRqst( 1 ),
    flags( flags ), target( nullptr ), procReq() {
  for( uint32_t i = 0; i < Size; i++ ) {
    membuf.push_back( buffer[i] );
  }
}

RevMemOp::RevMemOp(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, void* target, MemOp Op, RevFlag flags
)
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( 0 ), SplitRqst( 1 ),
    flags( flags ), target( target ), procReq() {
  for( uint32_t i = 0; i < Size; i++ ) {
    membuf.push_back( buffer[i] );
  }
}

RevMemOp::RevMemOp(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, std::vector<uint8_t> buffer, MemOp Op, RevFlag flags
)
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( 0 ), SplitRqst( 1 ),
    membuf( std::move( buffer ) ), flags( flags ), target( nullptr ), procReq() {}

RevMemOp::RevMemOp(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, uint32_t CustomOpc, MemOp Op, RevFlag flags
)
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( CustomOpc ), SplitRqst( 1 ),
    flags( flags ), target( target ), procReq() {}

RevMemOp::RevMemOp(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, uint32_t CustomOpc, MemOp Op, RevFlag flags
)
  : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Inv( false ), Op( Op ), CustomOpc( CustomOpc ), SplitRqst( 1 ),
    flags( flags ), target( nullptr ), procReq() {
  for( uint32_t i = 0; i < Size; i++ ) {
    membuf.push_back( (uint8_t) ( buffer[i] ) );
  }
}

// ---------------------------------------------------------------
// RevMemCtrl
// ---------------------------------------------------------------
RevMemCtrl::RevMemCtrl( ComponentId_t id, const Params& params ) : SubComponent( id ), output( nullptr ) {

  uint32_t verbosity = params.find<uint32_t>( "verbose" );
  output             = new SST::Output( "[RevMemCtrl @t]: ", verbosity, 0, SST::Output::STDOUT );
}

RevMemCtrl::~RevMemCtrl() {
  delete output;
}

// ---------------------------------------------------------------
// RevBasicMemCtrl
// ---------------------------------------------------------------
RevBasicMemCtrl::RevBasicMemCtrl( ComponentId_t id, const Params& params )
  : RevMemCtrl( id, params ), memIface( nullptr ), stdMemHandlers( nullptr ), hasCache( false ), lineSize( 0 ), max_loads( 64 ),
    max_stores( 64 ), max_flush( 64 ), max_llsc( 64 ), max_readlock( 64 ), max_writeunlock( 64 ), max_custom( 64 ), max_ops( 2 ),
    num_read( 0 ), num_write( 0 ), num_flush( 0 ), num_llsc( 0 ), num_readlock( 0 ), num_writeunlock( 0 ), num_custom( 0 ),
    num_fence( 0 ) {

  stdMemHandlers        = new RevStdMemHandlers( this, output );

  std::string ClockFreq = params.find<std::string>( "clock", "1Ghz" );

  max_loads             = params.find<uint32_t>( "max_loads", 64 );
  max_stores            = params.find<uint32_t>( "max_stores", 64 );
  max_flush             = params.find<uint32_t>( "max_flush", 64 );
  max_llsc              = params.find<uint32_t>( "max_llsc", 64 );
  max_readlock          = params.find<uint32_t>( "max_readlock", 64 );
  max_writeunlock       = params.find<uint32_t>( "max_writeunlock", 64 );
  max_custom            = params.find<uint32_t>( "max_custom", 64 );
  max_ops               = params.find<uint32_t>( "ops_per_cycle", 2 );

  rqstQ.reserve( max_ops );

  memIface = loadUserSubComponent<Interfaces::StandardMem>(
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

RevBasicMemCtrl::~RevBasicMemCtrl() {
  for( auto* p : rqstQ )
    delete p;
  delete stdMemHandlers;
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

void RevBasicMemCtrl::recordStat( MemCtrlStats Stat, uint64_t Data ) {
  if( Stat < MemCtrlStats::END )
    stats[size_t( Stat )]->addData( Data );
}

bool RevBasicMemCtrl::sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, bool Inv, RevFlag flags ) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, MemOp::MemOpFLUSH, flags );
  Op->setInv( Inv );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::FlushPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendREADRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, target, MemOp::MemOpREAD, flags );
  Op->setMemReq( req );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::ReadPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendWRITERequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, buffer, MemOp::MemOpWRITE, flags );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::WritePending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendAMORequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, void* target, const MemReq& req, RevFlag flags
) {

  if( Size == 0 )
    return true;

  // Check to see if our flags contain an atomic request
  if( RevFlagAtomic( flags ) == RevFlag::F_NONE ) {
    // not an atomic request
    return true;
  }

  // Create a memory operation for the AMO
  // Since this is a read-modify-write operation, the first RevMemOp
  // is a MemOp::MemOpREAD.
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, buffer, target, MemOp::MemOpREAD, flags );
  Op->setMemReq( req );

  // Store the first operation in the AMOTable.  When the read
  // response comes back, we will catch the response, perform
  // the MODIFY (using the operation in flags), then dispatch
  // a WRITE operation.
  auto tmp = std::make_tuple( Hart, buffer, target, flags, Op, false );
  AMOTable.insert( { Addr, tmp } );

  // We have the request created and recorded in the AMOTable
  // Push it onto the request queue
  rqstQ.push_back( Op );

  // now we record the stat for the particular AMO
  static const std::map<RevFlag, MemCtrlStats> table = {
  // clang-format off
    { RevFlag::F_AMOADD,   MemCtrlStats::AMOAddPending   },
    { RevFlag::F_AMOXOR,   MemCtrlStats::AMOXorPending   },
    { RevFlag::F_AMOAND,   MemCtrlStats::AMOAndPending   },
    { RevFlag::F_AMOOR,    MemCtrlStats::AMOOrPending    },
    { RevFlag::F_AMOMIN,   MemCtrlStats::AMOMinPending   },
    { RevFlag::F_AMOMAX,   MemCtrlStats::AMOMaxPending   },
    { RevFlag::F_AMOMIN,   MemCtrlStats::AMOMinuPending  },
    { RevFlag::F_AMOMAXU,  MemCtrlStats::AMOMaxuPending  },
    { RevFlag::F_AMOSWAP,  MemCtrlStats::AMOSwapPending  },
  // clang-format on
  };
  auto it = table.find( RevFlagAtomic( flags ) );
  if( it != table.end() )
    recordStat( it->second, 1 );
  return true;
}

bool RevBasicMemCtrl::sendREADLOCKRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, target, MemOp::MemOpREADLOCK, flags );
  Op->setMemReq( req );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::ReadLockPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendWRITELOCKRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, buffer, MemOp::MemOpWRITEUNLOCK, flags );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::WriteUnlockPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags ) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, MemOp::MemOpLOADLINK, flags );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::LoadLinkPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendSTORECONDRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, buffer, MemOp::MemOpSTORECOND, flags );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::StoreCondPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMREADRequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, uint32_t Opc, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, target, Opc, MemOp::MemOpCUSTOM, flags );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::CustomPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMWRITERequest(
  uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, uint32_t Opc, RevFlag flags
) {
  if( Size == 0 )
    return true;
  RevMemOp* Op = new RevMemOp( Hart, Addr, PAddr, Size, buffer, Opc, MemOp::MemOpCUSTOM, flags );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::CustomPending, 1 );
  return true;
}

bool RevBasicMemCtrl::sendFENCE( uint32_t Hart ) {
  RevMemOp* Op = new RevMemOp( Hart, 0, 0, 0, MemOp::MemOpFENCE, RevFlag::F_NONE );
  rqstQ.push_back( Op );
  recordStat( MemCtrlStats::FencePending, 1 );
  return true;
}

void RevBasicMemCtrl::processMemEvent( StandardMem::Request* ev ) {
  output->verbose( CALL_INFO, 15, 0, "Received memory request event\n" );
  if( ev == nullptr ) {
    output->fatal( CALL_INFO, -1, "Error : Received null memory event\n" );
  }
  ev->handle( stdMemHandlers );
}

void RevBasicMemCtrl::init( uint32_t phase ) {
  memIface->init( phase );

  // query the caching infrastructure
  if( phase == 1 ) {
    lineSize = uint32_t( memIface->getLineSize() );
    if( lineSize > 0 ) {
      output->verbose( CALL_INFO, 5, 0, "Detected cache layers; default line size=%" PRIu32 "\n", lineSize );
      hasCache = true;
    } else {
      output->verbose( CALL_INFO, 5, 0, "No cache detected; disabling caching\n" );
      hasCache = false;
    }
  }
}

void RevBasicMemCtrl::setup() {
  memIface->setup();
}

void RevBasicMemCtrl::finish() {}

bool RevBasicMemCtrl::isMemOpAvail(
  RevMemOp* Op,
  uint32_t& t_max_loads,
  uint32_t& t_max_stores,
  uint32_t& t_max_flush,
  uint32_t& t_max_llsc,
  uint32_t& t_max_readlock,
  uint32_t& t_max_writeunlock,
  uint32_t& t_max_custom
) {

  switch( Op->getOp() ) {
  case MemOp::MemOpREAD:
    if( t_max_loads < max_loads ) {
      t_max_loads++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpWRITE:
    if( t_max_stores < max_stores ) {
      t_max_stores++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpFLUSH:
    if( t_max_flush < max_flush ) {
      t_max_flush++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpREADLOCK:
    if( t_max_readlock < max_readlock ) {
      t_max_readlock++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpWRITEUNLOCK:
    if( t_max_writeunlock < max_writeunlock ) {
      t_max_writeunlock++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpLOADLINK:
    if( t_max_llsc < max_llsc ) {
      t_max_llsc++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpSTORECOND:
    if( t_max_llsc < max_llsc ) {
      t_max_llsc++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpCUSTOM:
    if( t_max_custom < max_custom ) {
      t_max_custom++;
      return true;
    }
    return false;
    break;
  case MemOp::MemOpFENCE: return true; break;
  default:
    output->fatal( CALL_INFO, -1, "Error : unknown memory operation type\n" );
    return false;
    break;
  }
  return false;
}

uint32_t RevBasicMemCtrl::getBaseCacheLineSize( uint64_t Addr, uint32_t Size ) const {

  bool     done          = false;
  uint64_t BaseCacheAddr = Addr;
  while( !done ) {
    if( BaseCacheAddr % lineSize == 0 ) {
      done = true;
    } else {
      BaseCacheAddr -= 1;
    }
  }

#ifdef _REV_DEBUG_
  std::cout << "not aligned to a base cache line" << std::endl;
  std::cout << "BaseCacheAddr = 0x" << std::hex << BaseCacheAddr << std::dec << std::endl;
  std::cout << "Addr          = 0x" << std::hex << Addr << std::dec << std::endl;
  std::cout << "lineSize      = " << (uint64_t) ( lineSize ) << std::endl;
#endif

  if( Addr == BaseCacheAddr ) {
    if( Size < lineSize ) {
      return Size;
    } else {
      return lineSize;
    }
  } else if( Addr + Size <= BaseCacheAddr + lineSize ) {
    // we stay within a single cache line
    return Size;
  } else {
    return uint32_t( BaseCacheAddr + lineSize - Addr );
  }
}

uint32_t RevBasicMemCtrl::getNumCacheLines( uint64_t Addr, uint32_t Size ) const {
  // if the cache is disabled, then return 1
  // eg, there is a 1-to-1 mapping of CPU memops to memory requests
  if( !hasCache )
    return 1;

  if( Addr % lineSize ) {
    if( (uint64_t) ( Size ) <= ( lineSize - ( Addr % lineSize ) ) ) {
      return 1;
    } else if( Size < lineSize ) {
      // this request is less than a cache line but
      // due to the offset, it spans two cache lines
      return 2;
    } else {
      return ( ( Size / lineSize ) + ( Addr % lineSize > 1 ) );
    }
  } else {
    // address is aligned already
    if( Size <= lineSize ) {
      return 1;
    } else {
      return ( Size / lineSize ) + 1;
    }
  }
}

bool RevBasicMemCtrl::buildCacheMemRqst( RevMemOp* op, bool& Success ) {
  Interfaces::StandardMem::Request* rqst     = nullptr;
  uint32_t                          NumLines = getNumCacheLines( op->getAddr(), op->getSize() );
  RevFlag                           TmpFlags = op->getStdFlags();

#ifdef _REV_DEBUG_
  std::cout << "building caching mem request for addr=0x" << std::hex << op->getAddr() << std::dec << "; NumLines = " << NumLines
            << "; Size = " << op->getSize() << std::endl;
#endif

  // first determine if we have enough request slots to service all the cache lines
  // if we don't have enough request slots, then requeue the entire RevMemOp
  switch( op->getOp() ) {
  case MemOp::MemOpREAD:
    if( ( max_loads - num_read ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpWRITE:
    if( ( max_stores - num_write ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpFLUSH:
    if( ( max_flush - num_flush ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpREADLOCK:
    if( ( max_readlock - num_readlock ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpWRITEUNLOCK:
    if( ( max_writeunlock - num_writeunlock ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpLOADLINK:
    if( ( max_llsc - num_llsc ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpSTORECOND:
    if( ( max_llsc - num_llsc ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  case MemOp::MemOpCUSTOM:
    if( ( max_custom - num_custom ) < NumLines ) {
      Success = false;
      return true;
    }
    break;
  default: return false; break;
  }

  Success = true;

#ifdef _REV_DEBUG_
  std::cout << "Found sufficient request slots for multi-line cache requests" << std::endl;
#endif

  // dispatch the base request
  // this the base address to the end of the first cache line
  // this prevents us from sending requests that span multiple cache lines

  op->setSplitRqst( NumLines );

  std::vector<uint8_t> tmpBuf = op->getBuf();
  std::vector<uint8_t> newBuf;
  uint32_t             BaseCacheLineSize = 0;
  if( NumLines > 1 ) {
    BaseCacheLineSize = getBaseCacheLineSize( op->getAddr(), op->getSize() );
  } else {
    BaseCacheLineSize = op->getSize();
  }
#ifdef _REV_DEBUG_
  std::cout << "base cache line request size = " << BaseCacheLineSize << std::endl;
#endif
  uint64_t curByte = 0;

  switch( op->getOp() ) {
  case MemOp::MemOpREAD:
#ifdef _REV_DEBUG_
    std::cout << "<<<< READ REQUEST >>>>" << std::endl;
#endif
    rqst = new Interfaces::StandardMem::Read(
      op->getAddr(), (uint64_t) ( BaseCacheLineSize ), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::ReadInFlight, 1 );
    num_read++;
    break;
  case MemOp::MemOpWRITE:
#ifdef _REV_DEBUG_
    std::cout << "<<<< WRITE REQUEST >>>>" << std::endl;
#endif
    for( uint32_t i = 0; i < BaseCacheLineSize; i++ ) {
      newBuf.push_back( tmpBuf[i] );
    }
    curByte = BaseCacheLineSize;
    rqst    = new Interfaces::StandardMem::Write(
      op->getAddr(), (uint64_t) ( BaseCacheLineSize ), newBuf, (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::WriteInFlight, 1 );
    num_write++;
    break;
  case MemOp::MemOpFLUSH:
    rqst = new Interfaces::StandardMem::FlushAddr(
      op->getAddr(), BaseCacheLineSize, op->getInv(), BaseCacheLineSize, (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::FlushInFlight, 1 );
    num_flush++;
    break;
  case MemOp::MemOpREADLOCK:
    rqst = new Interfaces::StandardMem::ReadLock(
      op->getAddr(), (uint64_t) ( BaseCacheLineSize ), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::ReadLockInFlight, 1 );
    num_readlock++;
    break;
  case MemOp::MemOpWRITEUNLOCK:
    for( uint32_t i = 0; i < BaseCacheLineSize; i++ ) {
      newBuf.push_back( tmpBuf[i] );
    }
    curByte = BaseCacheLineSize;
    rqst    = new Interfaces::StandardMem::WriteUnlock(
      op->getAddr(), (uint64_t) ( BaseCacheLineSize ), newBuf, false, (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::WriteUnlockInFlight, 1 );
    num_writeunlock++;
    break;
  case MemOp::MemOpLOADLINK:
    rqst = new Interfaces::StandardMem::LoadLink(
      op->getAddr(), (uint64_t) ( BaseCacheLineSize ), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::LoadLinkInFlight, 1 );
    num_llsc++;
    break;
  case MemOp::MemOpSTORECOND:
    for( uint32_t i = 0; i < BaseCacheLineSize; i++ ) {
      newBuf.push_back( tmpBuf[i] );
    }
    curByte = BaseCacheLineSize;
    rqst    = new Interfaces::StandardMem::StoreConditional(
      op->getAddr(), (uint64_t) ( BaseCacheLineSize ), newBuf, (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::StoreCondInFlight, 1 );
    num_llsc++;
    break;
  case MemOp::MemOpCUSTOM:
    // TODO: need more support for custom memory ops
    rqst = new Interfaces::StandardMem::CustomReq( nullptr, (StandardMem::Request::flags_t) TmpFlags );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::CustomInFlight, 1 );
    num_custom++;
    break;
  case MemOp::MemOpFENCE:
    // we should never get here with a FENCE operation
    // the FENCE is handled locally and never dispatch on the memIface
  default: return false; break;
  }

  // dispatch a request for each subsequent cache line
  newBuf.clear();
  uint64_t newBase   = op->getAddr() + BaseCacheLineSize;
  uint64_t bytesLeft = (uint64_t) ( op->getSize() ) - BaseCacheLineSize;
  uint32_t newSize   = 0;

  for( uint32_t i = 1; i < NumLines; i++ ) {
    // setup the adjusted size of the request
    if( bytesLeft < lineSize ) {
      newSize = uint32_t( bytesLeft );
    } else {
      newSize = uint32_t( lineSize );
    }

    // clear the adjusted buffer
    newBuf.clear();

    switch( op->getOp() ) {
    case MemOp::MemOpREAD:
      rqst = new Interfaces::StandardMem::Read( newBase, newSize, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::ReadInFlight, 1 );
      num_read++;
      break;
    case MemOp::MemOpWRITE:
      for( auto j = curByte; j < curByte + newSize; j++ ) {
        newBuf.push_back( tmpBuf[j] );
      }
      curByte += newSize;
      rqst = new Interfaces::StandardMem::Write( newBase, newSize, newBuf, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::WriteInFlight, 1 );
      num_write++;
      break;
    case MemOp::MemOpFLUSH:
      rqst =
        new Interfaces::StandardMem::FlushAddr( newBase, newSize, op->getInv(), newSize, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::FlushInFlight, 1 );
      num_flush++;
      break;
    case MemOp::MemOpREADLOCK:
      rqst = new Interfaces::StandardMem::ReadLock( newBase, newSize, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::ReadLockInFlight, 1 );
      num_readlock++;
      break;
    case MemOp::MemOpWRITEUNLOCK:
      for( auto j = curByte; j < curByte + newSize; j++ ) {
        newBuf.push_back( tmpBuf[j] );
      }
      curByte += newSize;
      rqst = new Interfaces::StandardMem::WriteUnlock( newBase, newSize, newBuf, false, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::WriteUnlockInFlight, 1 );
      num_writeunlock++;
      break;
    case MemOp::MemOpLOADLINK:
      rqst = new Interfaces::StandardMem::LoadLink( newBase, newSize, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::LoadLinkInFlight, 1 );
      num_llsc++;
      break;
    case MemOp::MemOpSTORECOND:
      for( auto j = curByte; j < curByte + newSize; j++ ) {
        newBuf.push_back( tmpBuf[j] );
      }
      curByte += newSize;
      rqst = new Interfaces::StandardMem::StoreConditional( newBase, newSize, newBuf, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::StoreCondInFlight, 1 );
      num_llsc++;
      break;
    case MemOp::MemOpCUSTOM:
      // TODO: need more support for custom memory ops
      rqst = new Interfaces::StandardMem::CustomReq( nullptr, (StandardMem::Request::flags_t) TmpFlags );
      requests.push_back( rqst->getID() );
      outstanding[rqst->getID()] = op;
      memIface->send( rqst );
      recordStat( MemCtrlStats::CustomInFlight, 1 );
      num_custom++;
      break;
    case MemOp::MemOpFENCE:
      // we should never get here with a FENCE operation
      // the FENCE is handled locally and never dispatch on the memIface
    default: return false; break;
    }  // end case
    bytesLeft -= newSize;
    newBase += newSize;
  }  // end for
  return true;
}

bool RevBasicMemCtrl::buildRawMemRqst( RevMemOp* op, RevFlag TmpFlags ) {
  Interfaces::StandardMem::Request* rqst = nullptr;

#ifdef _REV_DEBUG_
  std::cout << "building raw mem request for addr=0x" << std::hex << op->getAddr() << std::dec << "; Flags = 0x" << std::hex
            << (StandardMem::Request::flags_t) TmpFlags << std::dec << std::endl;
#endif

  switch( op->getOp() ) {
  case MemOp::MemOpREAD:
    rqst =
      new Interfaces::StandardMem::Read( op->getAddr(), (uint64_t) ( op->getSize() ), (StandardMem::Request::flags_t) TmpFlags );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::ReadInFlight, 1 );
    num_read++;
    break;
  case MemOp::MemOpWRITE:
    rqst = new Interfaces::StandardMem::Write(
      op->getAddr(), (uint64_t) ( op->getSize() ), op->getBuf(), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::WriteInFlight, 1 );
    num_write++;
    break;
  case MemOp::MemOpFLUSH:
    rqst = new Interfaces::StandardMem::FlushAddr(
      op->getAddr(), op->getSize(), op->getInv(), op->getSize(), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::FlushInFlight, 1 );
    num_flush++;
    break;
  case MemOp::MemOpREADLOCK:
    rqst = new Interfaces::StandardMem::ReadLock(
      op->getAddr(), (uint64_t) ( op->getSize() ), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::ReadLockInFlight, 1 );
    num_readlock++;
    break;
  case MemOp::MemOpWRITEUNLOCK:
    rqst = new Interfaces::StandardMem::WriteUnlock(
      op->getAddr(), (uint64_t) ( op->getSize() ), op->getBuf(), false, (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::WriteUnlockInFlight, 1 );
    num_writeunlock++;
    break;
  case MemOp::MemOpLOADLINK:
    rqst = new Interfaces::StandardMem::LoadLink(
      op->getAddr(), (uint64_t) ( op->getSize() ), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::LoadLinkInFlight, 1 );
    num_llsc++;
    break;
  case MemOp::MemOpSTORECOND:
    rqst = new Interfaces::StandardMem::StoreConditional(
      op->getAddr(), (uint64_t) ( op->getSize() ), op->getBuf(), (StandardMem::Request::flags_t) TmpFlags
    );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::StoreCondInFlight, 1 );
    num_llsc++;
    break;
  case MemOp::MemOpCUSTOM:
    // TODO: need more support for custom memory ops
    rqst = new Interfaces::StandardMem::CustomReq( nullptr, (StandardMem::Request::flags_t) TmpFlags );
    requests.push_back( rqst->getID() );
    outstanding[rqst->getID()] = op;
    memIface->send( rqst );
    recordStat( MemCtrlStats::CustomInFlight, 1 );
    num_custom++;
    break;
  case MemOp::MemOpFENCE:
    // we should never get here with a FENCE operation
    // the FENCE is handled locally and never dispatch on the memIface
  default: return false; break;
  }
  return true;
}

bool RevBasicMemCtrl::buildStandardMemRqst( RevMemOp* op, bool& Success ) {
  if( !op ) {
    return false;
  }

#ifdef _REV_DEBUG_
  std::cout << "building mem request for addr=0x" << std::hex << op->getAddr() << std::dec << "; flags = 0x" << std::hex
            << (StandardMem::Request::flags_t) op->getFlags() << std::dec << std::endl;
  if( !lineSize )
    std::cout << "WARNING: lineSize == 0!" << std::endl;
  else if( op->getAddr() % lineSize )
    std::cout << "WARNING: address is not cache aligned!" << std::endl;
  if( !isCacheable( op->getFlags() ) )
    std::cout << "WARNING: operation is not cache-able!" << std::endl;
#endif

  // ---------------------------------------------------------
  // Cache Handler Logic
  // ---------------------------------------------------------
  // There are five potential scenarios:
  // 1. Caching is disabled (no L1 cache detected)
  // 2. Addr = Cache Aligned && Size <= LineSize
  // 3. Addr = Cache Aligned && Size > LineSize
  // 4. Addr = !Cache Aligned && Size <= LineSize
  // 5. Addr = !Cache Aligned && Size > LineSize
  //
  // We handle these by adjusting:
  // 1. the number of cache lines to request
  // 2. the base address of each request (cache aligned)
  //
  // If caching is disabled, then the number of cache lines is
  // ALWAYS 1 and we dispatch a single memory requests per
  // RevMemOp
  // ---------------------------------------------------------
  if( hasCache ) {
    if( isCacheable( op->getFlags() ) ) {
      // cache is enabled and we want to cache the request
      return buildCacheMemRqst( op, Success );
    } else {
      // cache is enabled but the request says not to cache the data
      Success = true;
      return buildRawMemRqst( op, op->getStdFlags() );
    }
  } else {
    // no cache enabled
    Success = true;
    return buildRawMemRqst( op, op->getNonCacheFlags() );
  }
}

bool RevBasicMemCtrl::isAQ( uint32_t Slot, uint32_t Hart ) {
  if( AMOTable.size() == 0 ) {
    return false;
  } else if( Slot == 0 ) {
    return false;
  }

  // search all preceding slots for an AMO from the same Hart
  for( uint32_t i = 0; i < Slot; i++ ) {
    if( RevFlagAtomic( rqstQ[i]->getFlags() ) != RevFlag::F_NONE && rqstQ[i]->getHart() == rqstQ[Slot]->getHart() ) {
      if( RevFlagHas( rqstQ[i]->getFlags(), RevFlag::F_AQ ) ) {
        // this implies that we found a preceding request in the request queue
        // that was 1) an AMO and 2) came from the same HART as 'slot'
        // and 3) had the AQ flag set;
        // we must wait until this operation clears before this particular
        // request can proceed
        return true;
      }
    }
  }
  return false;
}

bool RevBasicMemCtrl::isRL( uint32_t Slot, uint32_t Hart ) {
  if( AMOTable.size() == 0 ) {
    return false;
  } else if( Slot == 0 ) {
    return false;
  }

  if( RevFlagAtomic( rqstQ[Slot]->getFlags() ) != RevFlag::F_NONE && RevFlagHas( rqstQ[Slot]->getFlags(), RevFlag::F_RL ) ) {
    // this is an AMO, check to see if there are other ops from the same
    // HART in flight
    for( uint32_t i = 0; i < Slot; i++ ) {
      if( rqstQ[i]->getHart() == rqstQ[Slot]->getHart() ) {
        // this implies that the same Hart has preceding memory ops
        // in which case, we can't dispatch this AMO until they clear
        return true;
      }
    }
  }
  return false;
}

bool RevBasicMemCtrl::isPendingAMO( uint32_t Slot ) {
  return ( isAQ( Slot, rqstQ[Slot]->getHart() ) || isRL( Slot, rqstQ[Slot]->getHart() ) );
}

bool RevBasicMemCtrl::processNextRqst(
  uint32_t& t_max_loads,
  uint32_t& t_max_stores,
  uint32_t& t_max_flush,
  uint32_t& t_max_llsc,
  uint32_t& t_max_readlock,
  uint32_t& t_max_writeunlock,
  uint32_t& t_max_custom,
  uint32_t& t_max_ops
) {
  if( rqstQ.size() == 0 ) {
    // nothing to do, saturate and exit this cycle
    t_max_ops = max_ops;
    return true;
  }

  bool success = false;

  // retrieve the next candidate memory operation
  for( uint32_t i = 0; i < rqstQ.size(); i++ ) {
    RevMemOp* op = rqstQ[i];
    if( isMemOpAvail( op, t_max_loads, t_max_stores, t_max_flush, t_max_llsc, t_max_readlock, t_max_writeunlock, t_max_custom ) ) {

      // op is good to execute, build a StandardMem packet
      t_max_ops++;

      if( op->getOp() == MemOp::MemOpFENCE ) {
        // time to fence!
        // saturate and exit this cycle
        // no need to build a StandardMem request
        t_max_ops = max_ops;
        rqstQ.erase( rqstQ.begin() + i );
        num_fence += 1;
        delete op;
        return true;
      }

      // determine if we have any AMOs that would prevent us
      // from dispatching this request.  if this returns 'true'
      // then we can't dispatch the request.  note that
      // we do this after processing FENCE requests
      if( isPendingAMO( i ) ) {
        t_max_ops = max_ops;
        return true;
      }

      // build a StandardMem request
      if( !buildStandardMemRqst( op, success ) ) {
        output->fatal( CALL_INFO, -1, "Error : failed to build memory request" );
        return false;
      }

      // sent the request, remove it
      if( success ) {
        rqstQ.erase( rqstQ.begin() + i );
      } else {
        // go ahead and max out our current request window
        // otherwise, this request for induce an infinite loop
        // we also leave the current (failed) request in the queue
        t_max_ops = max_ops;
      }

      return true;
    }
  }

  // if we reach this point, then we've attempted to
  // process all the potential requests.  none exist
  // that can be dispatched at this time.
  t_max_ops = max_ops;

#ifdef _REV_DEBUG_
  for( uint32_t i = 0; i < rqstQ.size(); i++ ) {
    std::cout << "rqstQ[" << i << "] = " << rqstQ[i]->getOp() << " @ 0x" << std::hex << rqstQ[i]->getAddr() << std::dec
              << "; physAddr = 0x" << std::hex << rqstQ[i]->getPhysAddr() << std::dec << std::endl;
  }
#endif

  return true;
}

/// RevFlag: Handle flag response
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

uint32_t RevBasicMemCtrl::getNumSplitRqsts( RevMemOp* op ) const {
  return (uint32_t) std::count_if( outstanding.begin(), outstanding.end(), [op]( auto& x ) { return x.second == op; } );
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

  // Table mapping atomic operations to executable code
  // clang-format off
  static const std::map<RevFlag, std::function<void()>> table = {
    { RevFlag::F_AMOADD,   [&]{ *TmpTarget += TmpBuf; } },
    { RevFlag::F_AMOXOR,   [&]{ *TmpTarget ^= TmpBuf; } },
    { RevFlag::F_AMOAND,   [&]{ *TmpTarget &= TmpBuf; } },
    { RevFlag::F_AMOOR,    [&]{ *TmpTarget |= TmpBuf; } },
    { RevFlag::F_AMOSWAP,  [&]{ *TmpTarget  = TmpBuf; } },
    { RevFlag::F_AMOMIN,   [&]{ *TmpTarget  = std::min( *TmpTarget,  TmpBuf );  } },
    { RevFlag::F_AMOMAX,   [&]{ *TmpTarget  = std::max( *TmpTarget,  TmpBuf );  } },
    { RevFlag::F_AMOMINU,  [&]{ *TmpTargetU = std::min( *TmpTargetU, TmpBufU ); } },
    { RevFlag::F_AMOMAXU,  [&]{ *TmpTargetU = std::max( *TmpTargetU, TmpBufU ); } },
  };
  // clang-format on
  table.at( RevFlagAtomic( flags ) )();
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

void RevBasicMemCtrl::performAMOMemH( RevMemOp* Tmp ) {
  if( Tmp == nullptr ) {
    output->fatal( CALL_INFO, -1, "Error : AMOTable entry is null\n" );
  }

  RevFlag  flags = Tmp->getFlags();
  uint32_t size  = Tmp->getSize();

  // Perform the AMO operation on the already-loaded data
  auto newMem    = performAMO( flags, size, Tmp->getTarget(), &Tmp->getBuf()[0] );

  // copy the modified target data over to the buffer and build the memory request
  // this will write the value to memory
  std::vector<uint8_t> buffer;
  for( uint32_t i = 0; i < size; ++i )
    buffer.push_back( newMem.uc[i] );

  RevMemOp* Op =
    new RevMemOp( Tmp->getHart(), Tmp->getAddr(), Tmp->getPhysAddr(), size, std::move( buffer ), MemOp::MemOpWRITE, flags );

  // Retrieve the memory request object, but DO NOT mark the load
  // as complete.  The actual write response from the read-modify-write
  // process will mark the load as complete.  At this point, copy the
  // MemReq object to the new request
  Op->setMemReq( Tmp->getMemReq() );

  // insert a new entry into the AMO Table
  AMOTable.emplace(
    Op->getAddr(),
    std::make_tuple(
      Op->getHart(),
      nullptr,  // this can be null here since we don't need to modify the response
      Op->getTarget(),
      Op->getFlags(),
      Op,
      true
    )
  );
  rqstQ.push_back( Op );
}

template<typename RESP>
void RevBasicMemCtrl::handleResp( RESP* ev, const char* name, uint32_t* counter ) {
  auto id = ev->getID();
  auto it = std::find( requests.begin(), requests.end(), id );
  if( it == requests.end() )
    output->fatal( CALL_INFO, -1, "Error : found unknown %s\n", name );
  requests.erase( it );

  RevMemOp* op = outstanding[id];
  if( !op )
    output->fatal( CALL_INFO, -1, "RevMemOp is null in handle%s\n", name );

#ifdef _REV_DEBUG_
  std::cout << "handle" << name << " : id=" << id << " @Addr= 0x" << std::hex << op->getAddr() << std::dec << std::endl;
#endif

  // For read responses, handle split requests
  if constexpr( std::is_same_v<RESP, StandardMem::ReadResp> ) {

#ifdef _REV_DEBUG_
    for( uint32_t i = 0; i < op->getSize(); i++ ) {
      std::cout << "               : data[" << i << "] = " << (uint32_t) ( ev->data[i] ) << std::endl;
    }
    std::cout << "isOutstanding val = 0x" << std::hex << op->getMemReq().isOutstanding << std::dec << std::endl;
    std::cout << "Address of the target register = 0x" << std::hex << (uint64_t*) ( op->getTarget() ) << std::dec << std::endl;
#endif

    // determine if we have a split read request
    if( op->getSplitRqst() > 1 ) {
      // split request exists; determine how to handle it
      memcpy( static_cast<uint8_t*>( op->getTarget() ) + ( ev->pAddr - op->getAddr() ), &ev->data[0], ev->size );
    } else {
      // no split request exists; handle as normal
      memcpy( op->getTarget(), &ev->data[0], op->getSize() );
    }
  }

  // determine if we have a split request
  if( op->getSplitRqst() <= 1 || getNumSplitRqsts( op ) == 1 ) {
    // if this was not a split request or it was the last request to service, delete the op

    // Handle read and write responses
    if constexpr( std::is_same_v<RESP, StandardMem::ReadResp> || std::is_same_v<RESP, StandardMem::WriteResp> ) {

      // determine if we have an atomic request associated with this read/write operation
      bool isAMO = false;
      for( auto [i, end] = AMOTable.equal_range( op->getAddr() ); i != end; ++i ) {
        const auto& [hart, buffer, target, flags, memop, in] = i->second;
        if( memop == op ) {
          AMOTable.erase( i );  // erase the current entry so we can add a new one
          isAMO = true;
          break;
        }
      }

      if constexpr( std::is_same_v<RESP, StandardMem::ReadResp> ) {
        // handleReadResp

        // determine if we need to sign/zero extend or NaN-box the read value
        RevHandleFlagResp( op->getTarget(), op->getSize(), op->getFlags() );

        if( isAMO ) {
          performAMOMemH( op );  // perform the atomic operation and generate a WRITE request
        } else {
          op->getMemReq().MarkLoadComplete();  // for non-atomic operations, mark load complete
        }

      } else {
        // handleWriteResp

        // For atomic operations, mark the original load complete after write is completed
        if( isAMO )
          op->getMemReq().MarkLoadComplete();
      }
    }

    delete op;
  }

  outstanding.erase( id );
  delete ev;
  if( counter )
    --*counter;
}

bool RevBasicMemCtrl::clockTick( Cycle_t cycle ) {

  // check to see if the top request is a FENCE
  if( num_fence > 0 ) {
    if( ( num_read + num_write + num_llsc + num_readlock + num_writeunlock + num_custom ) != 0 ) {
      // waiting for the outstanding ops to clear
      recordStat( MemCtrlStats::FencePending, 1 );
      return false;
    } else {
      // clear the fence and continue processing
      num_fence--;
    }
  }

  // process the memory queue
  bool     done              = false;
  uint32_t t_max_ops         = 0;
  uint32_t t_max_loads       = 0;
  uint32_t t_max_stores      = 0;
  uint32_t t_max_flush       = 0;
  uint32_t t_max_llsc        = 0;
  uint32_t t_max_readlock    = 0;
  uint32_t t_max_writeunlock = 0;
  uint32_t t_max_custom      = 0;

  while( !done ) {
    if( !processNextRqst(
          t_max_loads, t_max_stores, t_max_flush, t_max_llsc, t_max_readlock, t_max_writeunlock, t_max_custom, t_max_ops
        ) ) {
      // error occurred
      output->fatal( CALL_INFO, -1, "Error : failed to process next memory request" );
    }

    if( t_max_ops == max_ops ) {
      done = true;
    }
  }

  return false;
}

}  // namespace SST::RevCPU

// EOF
