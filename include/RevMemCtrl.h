//
// _RevMemCtrl_h_//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVMEMCTRL_H_
#define _SST_REVCPU_REVMEMCTRL_H_

// -- C++ Headers
#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// -- SST Headers
#include "SST.h"

// -- RevCPU Headers
#include "RevCommon.h"
#include "RevFlag.h"
#include "RevInstHelpers.h"

namespace SST::RevCPU {

/// AMO data union
union AMOData {
  uint8_t       u8;
  uint16_t      u16;
  uint32_t      u32;
  uint64_t      u64;
  float         f;
  double        d;
  unsigned char uc[8];
};

// ----------------------------------------
// RevMemOp
// ----------------------------------------
class RevMemOp {
  uint32_t refCount = 0;  ///< RevMemOp: number of memory requests referring to this op

  // Mandatory parameters
  const MemOp    Op;     ///< RevMemOp: target memory operation
  const uint32_t Hart;   ///< RevMemOp: RISC-V Hart
  const uint64_t Addr;   ///< RevMemOp: address
  const uint64_t PAddr;  ///< RevMemOp: physical address (for RevMem I/O)
  const uint32_t Size;   ///< RevMemOp: size of the memory operation in bytes
  const RevFlag  flags;  ///< RevMemOp: request flags

  // Optional parameters
  void* const                target{};     ///< RevMemOp: target register pointer
  const std::vector<uint8_t> membuf{};     ///< RevMemOp: buffer
  const bool                 Inv{};        ///< RevMemOp: flush operation invalidate flag
  const uint32_t             CustomOpc{};  ///< RevMemOp: custom memory opcode
  const MemReq               procReq{};    ///< RevMemOp: original request from RevCore

public:
  // clang-format off

  /// RevMemOp constructor
  RevMemOp( MemOp Op, uint32_t Hart, uint64_t Addr = 0, uint64_t PAddr = 0, uint32_t Size = 0, RevFlag flags = RevFlag::F_NONE, bool Inv = false )
    : Op( Op ), Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), flags( flags ), Inv( Inv ) {}

  /// RevMemOp overloaded constructor
  RevMemOp( MemOp Op, uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, const uint8_t* buffer )
    : Op( Op ), Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), flags( flags ), membuf( buffer, buffer + Size ) {}

  /// RevMemOp constructor
  RevMemOp( MemOp Op, uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, MemReq req = {} )
    : Op( Op ), Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), flags( flags ), target( target ), procReq( std::move( req ) ) {}

  /// RevMemOp constructor
  RevMemOp( MemOp Op, uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, const uint8_t* buffer, MemReq req )
    : Op( Op ), Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), flags( flags ), target( target ), membuf( buffer, buffer + Size ), procReq( std::move( req ) ) {}

  /// RevMemOp overloaded constructor
  RevMemOp( MemOp Op, uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, uint32_t CustomOpc )
    : Op( Op ), Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), flags( flags ), target( target ), CustomOpc( CustomOpc ) {}

  // clang-format on

  /// RevMemOp default destructor
  ~RevMemOp()                            = default;

  /// Disallow copying and assignment
  RevMemOp( const RevMemOp& )            = delete;
  RevMemOp& operator=( const RevMemOp& ) = delete;

  /// RevMemOp: retrieve the memory operation type
  auto getOp() const { return Op; }

  /// RevMemOp: retrieve the custom opcode
  auto getCustomOpc() const { return CustomOpc; }

  /// RevMemOp: retrieve the target address
  auto getAddr() const { return Addr; }

  /// RevMemOp: retrieve the target physical address
  auto getPhysAddr() const { return PAddr; }

  /// RevMemOp: retrieve the size of the request
  auto getSize() const { return Size; }

  /// RevMemOp: retrieve the memory buffer
  auto getBuf() const { return &membuf[0]; }

  /// RevMemOp: retrieve the memory operation flags
  auto getFlags() const { return flags; }

  /// RevMemOp: retrieve the standard set of memory flags for MemEventBase
  auto getStdFlags() const { return RevFlag{ safe_static_cast<uint32_t>( flags ) & 0xFFFF }; }

  /// RevMemOp: retrieve the flags for MemEventBase without caching enable
  auto getNonCacheFlags() const { return RevFlag{ safe_static_cast<uint32_t>( flags ) & 0xFFFD }; }

  /// RevMemOp: retrieve the invalidate flag
  auto getInv() const { return Inv; }

  /// RevMemOp: retrieve the target address
  auto getTarget() const { return target; }

  /// RevMemOp: retrieve the hart
  auto getHart() const { return Hart; }

  /// RevMemOp: Get the originating proc memory request
  auto& getMemReq() const { return procReq; }

  /// RqstCount: Get the number of requests referring to this op
  auto& rqstCount() { return refCount; }
};

// ----------------------------------------
// RevMemCtrl
// ----------------------------------------
class RevMemCtrl : public SST::SubComponent {
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API( SST::RevCPU::RevMemCtrl )

  SST_ELI_DOCUMENT_PARAMS( { "verbose", "Set the verbosity of output for the memory controller", "0" } )

  /// RevMemCtrl: constructor
  RevMemCtrl( ComponentId_t id, const Params& params );

  /// RevMemCtrl: initialization function
  void init( uint32_t phase ) override                                                                                  = 0;

  /// RevMemCtrl: setup function
  void setup() override                                                                                                 = 0;

  /// RevMemCtrl: finish function
  void finish() override                                                                                                = 0;

  /// RevMemCtrl: determines if outstanding requests exist
  virtual bool outstandingRqsts() const                                                                                 = 0;

  /// RevMemCtrl: send flush request
  virtual bool sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, bool Inv ) = 0;

  /// RevMemCtrl: send a read request
  virtual bool
    sendREADRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, MemReq req )    = 0;

  /// RevMemCtrl: send a write request
  virtual bool sendWRITERequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer ) = 0;

  /// RevMemCtrl: send an AMO request
  virtual bool sendAMORequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag Flags, uint8_t* Buffer, void* Target, MemReq Req
  ) = 0;

  /// RevMemCtrl: send a readlock request
  virtual bool sendREADLOCKRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flagvs, void* target, MemReq req
  ) = 0;

  /// RevMemCtrl: send a writelock request
  virtual bool
    sendWRITELOCKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer ) = 0;

  /// RevMemCtrl: send a loadlink request
  virtual bool sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags )        = 0;

  /// RevMemCtrl: send a storecond request
  virtual bool
    sendSTORECONDRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer ) = 0;

  /// RevMemCtrl: send an void custom read memory request
  virtual bool sendCUSTOMREADRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, uint32_t Opc
  ) = 0;

  /// RevMemCtrl: send a custom write request
  virtual bool sendCUSTOMWRITERequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer, uint32_t Opc
  )                                           = 0;

  /// RevMemCtrl: send a FENCE request
  virtual bool sendFENCE( uint32_t Hart )     = 0;

  /// RevMemCtrl: returns the cache line size
  virtual uint32_t getLineSize() const        = 0;

  /// Assign processor tracer
  virtual void setTracer( RevTracer* tracer ) = 0;

protected:
  const uint32_t                     verbose;  ///< RevMemCtrl: verbosity level
  const std::unique_ptr<SST::Output> output;   ///< RevMemCtrl: sst output object

};  // class RevMemCtrl

// ----------------------------------------
// RevBasicMemCtrl
// ----------------------------------------
class RevBasicMemCtrl final : public RevMemCtrl {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
    RevBasicMemCtrl,
    "revcpu",
    "RevBasicMemCtrl",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "RISC-V Rev basic memHierachy controller",
    SST::RevCPU::RevMemCtrl
  )

  // clang-format off
  SST_ELI_DOCUMENT_PARAMS({ "verbose",        "Set the verbosity of output for the memory controller",       "0" },
                          { "clock",          "Sets the clock frequency of the memory conroller",         "1Ghz" },
                          { "max_loads",      "Sets the maximum number of outstanding loads",               "64" },
                          { "max_stores",     "Sets the maximum number of outstanding stores",              "64" },
                          { "max_flush",      "Sets the maxmium number of oustanding flush events",         "64" },
                          { "max_llsc",       "Sets the maximum number of outstanding LL/SC events",        "64" },
                          { "max_readlock",   "Sets the maxmium number of outstanding readlock events",     "64" },
                          { "max_writeunlock","Sets the maximum number of outstanding writeunlock events",  "64" },
                          { "max_custom",     "Sets the maximum number of outstanding custom events",       "64" },
                          { "ops_per_cycle",  "Sets the maximum number of operations to issue per cycle",    "2" },
    )

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS({ "memIface", "Set the interface to memory", "SST::Interfaces::StandardMem" })

  SST_ELI_DOCUMENT_PORTS()

  SST_ELI_DOCUMENT_STATISTICS(
    {"ReadInFlight",        "Counts the number of reads in flight",              "count", 1},
    {"ReadPending",         "Counts the number of reads pending",                "count", 1},
    {"ReadBytes",           "Counts the number of bytes read",                   "bytes", 1},
    {"WriteInFlight",       "Counts the number of writes in flight",             "count", 1},
    {"WritePending",        "Counts the number of writes pending",               "count", 1},
    {"WriteBytes",          "Counts the number of bytes written",                "bytes", 1},
    {"FlushInFlight",       "Counts the number of flushes in flight",            "count", 1},
    {"FlushPending",        "Counts the number of flushes pending",              "count", 1},
    {"ReadLockInFlight",    "Counts the number of readlocks in flight",          "count", 1},
    {"ReadLockPending",     "Counts the number of readlocks pending",            "count", 1},
    {"ReadLockBytes",       "Counts the number of readlock bytes read",          "bytes", 1},
    {"WriteUnlockInFlight", "Counts the number of write unlocks in flight",      "count", 1},
    {"WriteUnlockPending",  "Counts the number of write unlocks pending",        "count", 1},
    {"WriteUnlockBytes",    "Counts the number of write unlock bytes written",   "bytes", 1},
    {"LoadLinkInFlight",    "Counts the number of loadlinks in flight",          "count", 1},
    {"LoadLinkPending",     "Counts the number of loadlinks pending",            "count", 1},
    {"StoreCondInFlight",   "Counts the number of storeconds in flight",         "count", 1},
    {"StoreCondPending",    "Counts the number of storeconds pending",           "count", 1},
    {"CustomInFlight",      "Counts the number of custom commands in flight",    "count", 1},
    {"CustomPending",       "Counts the number of custom commands pending",      "count", 1},
    {"CustomBytes",         "Counts the number of bytes in custom transactions", "bytes", 1},
    {"FencePending",        "Counts the number of fence operations pending",     "count", 1},
    {"AMOAddBytes",         "Counts the number of bytes in AMOAdd transactions", "bytes", 1},
    {"AMOAddPending",       "Counts the number of AMOAdd operations pending",    "count", 1},
    {"AMOXorBytes",         "Counts the number of bytes in AMOXor transactions", "bytes", 1},
    {"AMOXorPending",       "Counts the number of AMOXor operations pending",    "count", 1},
    {"AMOAndBytes",         "Counts the number of bytes in AMOAnd transactions", "bytes", 1},
    {"AMOAndPending",       "Counts the number of AMOAnd operations pending",    "count", 1},
    {"AMOOrBytes",          "Counts the number of bytes in AMOOr transactions",  "bytes", 1},
    {"AMOOrPending",        "Counts the number of AMOOr operations pending",     "count", 1},
    {"AMOMinBytes",         "Counts the number of bytes in AMOMin transactions", "bytes", 1},
    {"AMOMinPending",       "Counts the number of AMOMin operations pending",    "count", 1},
    {"AMOMaxBytes",         "Counts the number of bytes in AMOMax transactions", "bytes", 1},
    {"AMOMaxPending",       "Counts the number of AMOMax operations pending",    "count", 1},
    {"AMOMinuBytes",        "Counts the number of bytes in AMOMinu transactions","bytes", 1},
    {"AMOMinuPending",      "Counts the number of AMOMinu operations pending",   "count", 1},
    {"AMOMaxuBytes",        "Counts the number of bytes in AMOMaxu transactions","bytes", 1},
    {"AMOMaxuPending",      "Counts the number of AMOMaxu operations pending",   "count", 1},
    {"AMOSwapBytes",        "Counts the number of bytes in AMOSwap transactions","bytes", 1},
    {"AMOSwapPending",      "Counts the number of AMOSwap operations pending",   "count", 1},
    )

  // clang-format on

  enum class MemCtrlStats : uint32_t {
    ReadInFlight,
    ReadPending,
    ReadBytes,
    WriteInFlight,
    WritePending,
    WriteBytes,
    FlushInFlight,
    FlushPending,
    ReadLockInFlight,
    ReadLockPending,
    ReadLockBytes,
    WriteUnlockInFlight,
    WriteUnlockPending,
    WriteUnlockBytes,
    LoadLinkInFlight,
    LoadLinkPending,
    StoreCondInFlight,
    StoreCondPending,
    CustomInFlight,
    CustomPending,
    CustomBytes,
    FencePending,
    AMOAddBytes,
    AMOAddPending,
    AMOXorBytes,
    AMOXorPending,
    AMOAndBytes,
    AMOAndPending,
    AMOOrBytes,
    AMOOrPending,
    AMOMinBytes,
    AMOMinPending,
    AMOMaxBytes,
    AMOMaxPending,
    AMOMinuBytes,
    AMOMinuPending,
    AMOMaxuBytes,
    AMOMaxuPending,
    AMOSwapBytes,
    AMOSwapPending,
    END
  };

  /// RevBasicMemCtrl: constructor
  RevBasicMemCtrl( ComponentId_t id, const Params& params );

  /// RevBasicMemCtrl: destructor
  ~RevBasicMemCtrl() final                             = default;

  /// RevBasicMemCtrl: disallow copying and assignment
  RevBasicMemCtrl( const RevBasicMemCtrl& )            = delete;
  RevBasicMemCtrl& operator=( const RevBasicMemCtrl& ) = delete;

  /// RevBasicMemCtrl: initialization function
  void init( uint32_t phase ) final;

  /// RevBasicMemCtrl: setup function
  void setup() final { memIface->setup(); }

  /// RevBasicMemCtrl: finish function
  void finish() final {}

  /// RevBasicMemCtrl: clock tick function
  virtual bool clockTick( Cycle_t cycle );

  /// RevBasicMemCtrl: determines if outstanding requests exist
  bool outstandingRqsts() const final { return !outstanding.empty(); }

  /// RevBasicMemCtrl: returns the cache line size
  uint32_t getLineSize() const final { return lineSize; }

  /// RevBasicMemCtrl: memory event processing handler
  void processMemEvent( StandardMem::Request* ev );

  /// RevBasicMemCtrl: send a flush request
  bool sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, bool Inv ) final {
    return QRequest( MemCtrlStats::FlushPending, MemOp::MemOpFLUSH, Hart, Addr, PAddr, Size, flags, Inv );
  }

  /// RevBasicMemCtrl: send a read request
  bool
    sendREADRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, MemReq req ) final {
    return QRequest( MemCtrlStats::ReadPending, MemOp::MemOpREAD, Hart, Addr, PAddr, Size, flags, target, std::move( req ) );
  }

  /// RevBasicMemCtrl: send a write request
  bool sendWRITERequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer ) final {
    return QRequest( MemCtrlStats::WritePending, MemOp::MemOpWRITE, Hart, Addr, PAddr, Size, flags, buffer );
  }

  /// RevBasicMemCtrl: send an AMO request
  bool sendAMORequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* Buffer, void* Target, MemReq Req
  ) final;

  // RevBasicMemCtrl: send a readlock request
  bool sendREADLOCKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, MemReq req )
    final {
    return QRequest(
      MemCtrlStats::ReadLockPending, MemOp::MemOpREADLOCK, Hart, Addr, PAddr, Size, flags, target, std::move( req )
    );
  }

  // RevBasicMemCtrl: send a writelock request
  bool sendWRITELOCKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer ) final {
    return QRequest( MemCtrlStats::WriteUnlockPending, MemOp::MemOpWRITEUNLOCK, Hart, Addr, PAddr, Size, flags, buffer );
  }

  // RevBasicMemCtrl: send a loadlink request
  bool sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags ) final {
    return QRequest( MemCtrlStats::LoadLinkPending, MemOp::MemOpLOADLINK, Hart, Addr, PAddr, Size, flags );
  }

  // RevBasicMemCtrl: send a storecond request
  bool sendSTORECONDRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer ) final {
    return QRequest( MemCtrlStats::StoreCondPending, MemOp::MemOpSTORECOND, Hart, Addr, PAddr, Size, flags, buffer );
  }

  // RevBasicMemCtrl: send an void custom read memory request
  bool sendCUSTOMREADRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, void* target, uint32_t Opc
  ) final {
    return QRequest( MemCtrlStats::CustomPending, MemOp::MemOpCUSTOM, Hart, Addr, PAddr, Size, flags, target, Opc );
  }

  // RevBasicMemCtrl: send a custom write request
  bool sendCUSTOMWRITERequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, uint8_t* buffer, uint32_t Opc
  ) final {
    return QRequest( MemCtrlStats::CustomPending, MemOp::MemOpCUSTOM, Hart, Addr, PAddr, Size, flags, buffer, Opc );
  }

  // RevBasicMemCtrl: send a FENCE request
  bool sendFENCE( uint32_t Hart ) final;

  /// RevBasicMemCtrl: handle a response generally
  template<MemOp memOp, typename RESP>
  void handleResp( RESP* ev );

  /// RevBasicMemCtrl: perform an AMO on local data
  static AMOData performAMO( RevFlag flags, uint32_t size, void* target, const void* data );

  /// RevBasicMemCtrl: handle an AMO for the target READ+MODIFY+WRITE triplet
  void handleAMOResp( const std::shared_ptr<RevMemOp>& readOp );

  /// RevBasicMemCtrl: assign tracer pointer
  void setTracer( RevTracer* tracer ) final { Tracer = tracer; }

  /// RevBasicMemCtrl: handle flag response
  static void RevHandleFlagResp( void* target, size_t size, RevFlag flags );

  /// RevFlag: Perform an integer conversion
  template<typename SRC, typename DEST>
  static void RevConvertInt( void* target ) {
    SRC src;
    memcpy( &src, target, sizeof( src ) );
    DEST dest{ src };
    memcpy( target, &dest, sizeof( dest ) );
  }

private:
  // ----------------------------------------
  // RevStdMemHandlers
  // ----------------------------------------
  struct RevStdMemHandlers final : StandardMem::RequestHandler {
    /// RevStdMemHandlers: constructor
    explicit RevStdMemHandlers( RevBasicMemCtrl* Ctrl ) : RequestHandler( Ctrl->output.get() ), Ctrl( Ctrl ) {}

    /// RevStdMemHandlers: handlers
    void handle( StandardMem::ReadResp* ev ) final { Ctrl->handleResp<MemOp::MemOpREAD>( ev ); }

    void handle( StandardMem::WriteResp* ev ) final { Ctrl->handleResp<MemOp::MemOpWRITE>( ev ); }

    void handle( StandardMem::FlushResp* ev ) final { Ctrl->handleResp<MemOp::MemOpFLUSH>( ev ); }

    void handle( StandardMem::CustomResp* ev ) final { Ctrl->handleResp<MemOp::MemOpCUSTOM>( ev ); }

    void handle( StandardMem::InvNotify* ev ) final { Ctrl->handleResp<MemOp::MemOpINV>( ev ); }

    // ---------------------------------------------------------------
    // RevStdMemHandlers
    // ---------------------------------------------------------------

  private:
    RevBasicMemCtrl* const Ctrl;  ///< RevStdMemHandlers: memory controller object

  };  // class RevStdMemHandlers

  /// RevBasicMemCtrl: Queue a memory request
  template<typename... Ts>
  bool
    QRequest( MemCtrlStats stat, MemOp memOp, uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags, Ts&&... );

  /// RevBasicMemCtrl: process the next memory request
  bool processNextRqst();

  /// RevBasicMemCtrl: Add a new memory request
  void sendMemRqst( const std::shared_ptr<RevMemOp>& op, MemOp memOp, MemCtrlStats stat, StandardMem::Request* rqst );

  /// RevBasicMemCtrl: build a standard memory request
  bool buildStandardMemRqst( const std::shared_ptr<RevMemOp>& op );

  /// RevBasicMemCtrl: register statistics
  void registerStats();

  /// RevBasicMemCtrl: whether a memory operation is pending on previous memory operations
  bool isPendingAMO( const std::shared_ptr<RevMemOp>& op );

  /// RevBasicMemCtrl: inject statistics data for the target metric
  void recordStat( MemCtrlStats Stat, uint64_t Data = 1 ) {
    if( Stat < MemCtrlStats::END )
      stats[size_t( Stat )]->addData( Data );
  }

  void rqstQpush( const std::shared_ptr<RevMemOp>& op );

  // -- private data members
  RevTracer*                            Tracer{};    ///< tracer pointer
  StandardMem*                          memIface{};  ///< StandardMem memory interface
  bool                                  hasCache{};  ///< detects whether cache layers are present
  uint32_t                              lineSize{};  ///< cache line size
  MemOpParams                           memOpNum{};  ///< numbers in effect of memory parameters
  MemOpParams                           memOpMax{};  ///< maximums allowable of memory parameters
  std::vector<Statistic<uint64_t>*>     stats{};     ///< statistics vector
  std::queue<std::shared_ptr<RevMemOp>> rqstQ;

  //  std::multimap<uint32_t, std::shared_ptr<RevMemOp>> rqstQ;
  //  std::map<uint32_t, std::array<decltype(rqstQ)::iterator, 2>> rqstQit;

  ///< map of outstanding memory requests based on Hart id
  // note: std::multimap is used because it keeps iterators valid while std::unordered_multimap does not
  std::multimap<uint32_t, std::shared_ptr<RevMemOp>> hartOutstanding{};

  ///< map of outstanding memory requests based on Request id
  std::unordered_map<StandardMem::Request::id_t, decltype( hartOutstanding )::iterator> outstanding{};

  ///< StandardMem interface response handlers
  const std::unique_ptr<RevStdMemHandlers> stdMemHandlers{ new RevStdMemHandlers( this ) };

};  // RevBasicMemCtrl

}  // namespace SST::RevCPU

#endif  // _SST_REVCPU_REVMEMCTRL_H_
