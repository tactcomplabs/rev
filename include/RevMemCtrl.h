//
// _RevMemCtrl_h_
//
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
#include <memory>
#include <random>
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
public:
  /// RevMemOp constructor
  RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, MemOp Op, RevFlag flags )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), flags( flags ) {}

  /// RevMemOp constructor
  RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, MemOp Op, RevFlag flags )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), flags( flags ), target( target ) {}

  /// RevMemOp overloaded constructor
  RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, MemOp Op, RevFlag flags )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), membuf( buffer, buffer + Size ), flags( flags ) {}

  /// RevMemOp overloaded constructor
  RevMemOp(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, void* target, MemOp Op, RevFlag flags
  )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), membuf( buffer, buffer + Size ), flags( flags ),
      target( target ) {}

  /// RevMemOp overloaded constructor
  RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, std::vector<uint8_t> buffer, MemOp Op, RevFlag flags )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), membuf( std::move( buffer ) ), flags( flags ) {}

  /// RevMemOp overloaded constructor
  RevMemOp( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, uint32_t CustomOpc, MemOp Op, RevFlag flags )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), CustomOpc( CustomOpc ), flags( flags ), target( target ) {
  }

  /// RevMemOp overloaded constructor
  RevMemOp(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, uint32_t CustomOpc, MemOp Op, RevFlag flags
  )
    : Hart( Hart ), Addr( Addr ), PAddr( PAddr ), Size( Size ), Op( Op ), CustomOpc( CustomOpc ), membuf( buffer, buffer + Size ),
      flags( flags ) {}

  /// RevMemOp default destructor
  ~RevMemOp()                            = default;

  /// Disallow copying and assignment
  RevMemOp( const RevMemOp& )            = delete;
  RevMemOp& operator=( const RevMemOp& ) = delete;

  /// RevMemOp: retrieve the memory operation type
  MemOp getOp() const { return Op; }

  /// RevMemOp: retrieve the custom opcode
  uint32_t getCustomOpc() const { return CustomOpc; }

  /// RevMemOp: retrieve the target address
  uint64_t getAddr() const { return Addr; }

  /// RevMemOp: retrieve the target physical address
  uint64_t getPhysAddr() const { return PAddr; }

  /// RevMemOp: retrieve the size of the request
  uint32_t getSize() const { return Size; }

  /// RevMemOp: retrieve the memory buffer
  const std::vector<uint8_t>& getBuf() const { return membuf; }

  /// RevMemOp: retrieve the memory operation flags
  RevFlag getFlags() const { return flags; }

  /// RevMemOp: retrieve the standard set of memory flags for MemEventBase
  RevFlag getStdFlags() const { return RevFlag{ safe_static_cast<uint32_t>( flags ) & 0xFFFF }; }

  /// RevMemOp: retrieve the flags for MemEventBase without caching enable
  RevFlag getNonCacheFlags() const { return RevFlag{ safe_static_cast<uint32_t>( flags ) & 0xFFFD }; }

  /// RevMemOp: set the invalidate flag
  void setInv( bool I ) { Inv = I; }

  /// RevMemOp: set the hart
  void setHart( uint32_t H ) { Hart = H; }

  /// RevMemOp: set the originating memory request
  void setMemReq( const MemReq& req ) { procReq = req; }

  /// RevMemOp: retrieve the invalidate flag
  bool getInv() const { return Inv; }

  /// RevMemOp: retrieve the target address
  void* getTarget() const { return target; }

  /// RevMemOp: retrieve the hart
  uint32_t getHart() const { return Hart; }

  /// RevMemOp: Get the originating proc memory request
  const MemReq& getMemReq() const { return procReq; }

  /// RqstCount: Get the number of requests referring to this op
  auto& rqstCount() { return refCount; }

private:
  uint32_t             Hart{};       ///< RevMemOp: RISC-V Hart
  uint64_t             Addr{};       ///< RevMemOp: address
  uint64_t             PAddr{};      ///< RevMemOp: physical address (for RevMem I/O)
  uint32_t             Size{};       ///< RevMemOp: size of the memory operation in bytes
  bool                 Inv{};        ///< RevMemOp: flush operation invalidate flag
  MemOp                Op{};         ///< RevMemOp: target memory operation
  uint32_t             CustomOpc{};  ///< RevMemOp: custom memory opcode
  std::vector<uint8_t> membuf{};     ///< RevMemOp: buffer
  RevFlag              flags{};      ///< RevMemOp: request flags
  void*                target{};     ///< RevMemOp: target register pointer
  MemReq               procReq{};    ///< RevMemOp: original request from RevCore
  uint32_t             refCount{};   ///< RevMemOp: number of memory requests referring to this op
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

  /// RevMemCtrl: destructor
  virtual ~RevMemCtrl()                                                                                                 = default;

  /// RevMemCtrl: disallow copying and assignment
  RevMemCtrl( const RevMemCtrl& )                                                                                       = delete;
  RevMemCtrl& operator=( const RevMemCtrl& )                                                                            = delete;

  /// RevMemCtrl: initialization function
  void init( uint32_t phase ) override                                                                                  = 0;

  /// RevMemCtrl: setup function
  void setup() override                                                                                                 = 0;

  /// RevMemCtrl: finish function
  void finish() override                                                                                                = 0;

  /// RevMemCtrl: determines if outstanding requests exist
  virtual bool outstandingRqsts() const                                                                                 = 0;

  /// RevMemCtrl: send flush request
  virtual bool sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, bool Inv, RevFlag flags ) = 0;

  /// RevMemCtrl: send a read request
  virtual bool sendREADRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  ) = 0;

  /// RevMemCtrl: send a write request
  virtual bool
    sendWRITERequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags ) = 0;

  /// RevMemCtrl: send an AMO request
  virtual bool sendAMORequest(
    uint32_t       Hart,
    uint64_t       Addr,
    uint64_t       PAddr,
    uint32_t       Size,
    unsigned char* buffer,
    void*          target,
    const MemReq&  req,
    RevFlag        flags
  ) = 0;

  /// RevMemCtrl: send a readlock request
  virtual bool sendREADLOCKRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  ) = 0;

  /// RevMemCtrl: send a writelock request
  virtual bool
    sendWRITELOCKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags ) = 0;

  /// RevMemCtrl: send a loadlink request
  virtual bool sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags )              = 0;

  /// RevMemCtrl: send a storecond request
  virtual bool
    sendSTORECONDRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags ) = 0;

  /// RevMemCtrl: send an void custom read memory request
  virtual bool sendCUSTOMREADRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, uint32_t Opc, RevFlag flags
  ) = 0;

  /// RevMemCtrl: send a custom write request
  virtual bool sendCUSTOMWRITERequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, uint32_t Opc, RevFlag flags
  )                                                            = 0;

  /// RevMemCtrl: send a FENCE request
  virtual bool sendFENCE( uint32_t Hart )                      = 0;

  /// RevMemCtrl: handle a read response
  virtual void handleReadResp( StandardMem::ReadResp* ev )     = 0;

  /// RevMemCtrl: handle a write response
  virtual void handleWriteResp( StandardMem::WriteResp* ev )   = 0;

  /// RevMemCtrl: handle a flush response
  virtual void handleFlushResp( StandardMem::FlushResp* ev )   = 0;

  /// RevMemCtrl: handle a custom response
  virtual void handleCustomResp( StandardMem::CustomResp* ev ) = 0;

  /// RevMemCtrl: handle an invalidate response
  virtual void handleInvResp( StandardMem::InvNotify* ev )     = 0;

  /// RevMemCtrl: returns the cache line size
  virtual uint32_t getLineSize() const                         = 0;

  /// Assign processor tracer
  virtual void setTracer( RevTracer* tracer )                  = 0;

protected:
  std::unique_ptr<SST::Output> output{};  ///< RevMemCtrl: sst output object
  RevTracer*                   Tracer{};  ///< RevMemCtrl: tracer pointer

};  // class RevMemCtrl

// ----------------------------------------
// RevBasicMemCtrl
// ----------------------------------------
class RevBasicMemCtrl final : public RevMemCtrl {
public:
  // Memory operation parameters accessible with MemOp
  class MemOpParams {
    std::array<uint32_t, safe_static_cast<size_t>( MemOp::END )> paramVal{};

  public:
    // Access a memory operation parameter
    auto& operator[]( MemOp op ) {

      if( op == MemOp::MemOpSTORECOND )  // Combine LR+SC
        op = MemOp::MemOpLOADLINK;

      return paramVal.at( safe_static_cast<size_t>( op ) );
    }

    // const version
    const auto& operator[]( MemOp op ) const { return const_cast<MemOpParams&>( *this )[op]; }

    // default non-aggregate constructors and assignment operators
    explicit MemOpParams()                       = default;
    ~MemOpParams()                               = default;
    MemOpParams( const MemOpParams& )            = default;
    MemOpParams( MemOpParams&& )                 = default;
    MemOpParams& operator=( const MemOpParams& ) = default;
    MemOpParams& operator=( MemOpParams&& )      = default;
  };

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
  bool sendFLUSHRequest( uint32_t Hart, uint64_t Addr, uint64_t PAdr, uint32_t Size, bool Inv, RevFlag flags ) final;

  /// RevBasicMemCtrl: send a read request
  bool sendREADRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  ) final;

  /// RevBasicMemCtrl: send a write request
  bool sendWRITERequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags = RevFlag::F_NONE
  ) final;

  /// RevBasicMemCtrl: send an AMO request
  bool sendAMORequest(
    uint32_t       Hart,
    uint64_t       Addr,
    uint64_t       PAddr,
    uint32_t       Size,
    unsigned char* buffer,
    void*          target,
    const MemReq&  req,
    RevFlag        flags
  ) final;

  // RevBasicMemCtrl: send a readlock request
  bool sendREADLOCKRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  ) final;

  // RevBasicMemCtrl: send a writelock request
  bool
    sendWRITELOCKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags ) final;

  // RevBasicMemCtrl: send a loadlink request
  bool sendLOADLINKRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags ) final;

  // RevBasicMemCtrl: send a storecond request
  bool
    sendSTORECONDRequest( uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, RevFlag flags ) final;

  // RevBasicMemCtrl: send an void custom read memory request
  bool sendCUSTOMREADRequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, uint32_t Opc, RevFlag flags
  ) final;

  // RevBasicMemCtrl: send a custom write request
  bool sendCUSTOMWRITERequest(
    uint32_t Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, unsigned char* buffer, uint32_t Opc, RevFlag flags
  ) final;

  // RevBasicMemCtrl: send a FENCE request
  bool sendFENCE( uint32_t Hart ) final;

  // RevBasicMemCtrl: Whether an atomic request is associated with this read/write operation
  bool isAMO( const std::shared_ptr<RevMemOp>& op );

  /// RevBasicMemCtrl: handle a response generally
  template<typename RESP>
  void handleResp( RESP* ev, const char* name );

  /// RevBasicMemCtrl: handle a read response
  void handleReadResp( StandardMem::ReadResp* ev ) final {
    handleResp( ev, "ReadResp" );
    --memOpNum[MemOp::MemOpREAD];
  }

  /// RevBasicMemCtrl: handle a write response
  void handleWriteResp( StandardMem::WriteResp* ev ) final {
    handleResp( ev, "WriteResp" );
    --memOpNum[MemOp::MemOpWRITE];
  }

  /// RevBasicMemCtrl: handle a flush response
  void handleFlushResp( StandardMem::FlushResp* ev ) final {
    handleResp( ev, "FlushResp" );
    --memOpNum[MemOp::MemOpFLUSH];
  }

  /// RevBasicMemCtrl: handle a custom response
  void handleCustomResp( StandardMem::CustomResp* ev ) final {
    handleResp( ev, "CustomResp" );
    --memOpNum[MemOp::MemOpCUSTOM];
  }

  /// RevBasicMemCtrl: handle an invalidate response
  void handleInvResp( StandardMem::InvNotify* ev ) final { handleResp( ev, "InvResp" ); }

  /// RevBasicMemCtrl: perform an AMO on local data
  static AMOData performAMO( RevFlag flags, uint32_t size, void* target, const void* data );

  /// RevBasicMemCtrl: handle an AMO for the target READ+MODIFY+WRITE triplet
  void performAMOMemH( const std::shared_ptr<RevMemOp>& readOp );

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

protected:
  // ----------------------------------------
  // RevStdMemHandlers
  // ----------------------------------------
  struct RevStdMemHandlers final : Interfaces::StandardMem::RequestHandler {
    friend class RevBasicMemCtrl;

    /// RevStdMemHandlers: constructor
    RevStdMemHandlers( RevBasicMemCtrl* Ctrl, SST::Output* output )
      : Interfaces::StandardMem::RequestHandler( output ), Ctrl( Ctrl ) {}

    /// RevStdMemHandlers: destructor
    ~RevStdMemHandlers() final                               = default;

    /// RevStdMemHandlers: disallow copying and assignment
    RevStdMemHandlers( const RevStdMemHandlers& )            = delete;
    RevStdMemHandlers& operator=( const RevStdMemHandlers& ) = delete;

    void handle( StandardMem::ReadResp* ev ) final { Ctrl->handleReadResp( ev ); }

    void handle( StandardMem::WriteResp* ev ) final { Ctrl->handleWriteResp( ev ); }

    void handle( StandardMem::FlushResp* ev ) final { Ctrl->handleFlushResp( ev ); }

    void handle( StandardMem::CustomResp* ev ) final { Ctrl->handleCustomResp( ev ); }

    void handle( StandardMem::InvNotify* ev ) final { Ctrl->handleInvResp( ev ); }

    // ---------------------------------------------------------------
    // RevStdMemHandlers
    // ---------------------------------------------------------------

  private:
    RevBasicMemCtrl* Ctrl{};  ///< RevStdMemHandlers: memory controller object

  };  // class RevStdMemHandlers

private:
  /// RevBasicMemCtrl: process the next memory request
  bool processNextRqst( MemOpParams& t );

  /// RevBasicMemCtrl: Add a new memory request
  void addMemRqst( const std::shared_ptr<RevMemOp>& op, Interfaces::StandardMem::Request* rqst );

  /// RevBasicMemCtrl: build a standard memory request
  bool buildStandardMemRqst( const std::shared_ptr<RevMemOp>& op );

  /// RevBasicMemCtrl: build raw memory requests with a 1-to-1 mapping to RevMemOps'
  bool buildRawMemRqst( const std::shared_ptr<RevMemOp>& op, RevFlag Flags );

  /// RevBasicMemCtrl: build cache-aligned requests
  bool buildCacheMemRqst( const std::shared_ptr<RevMemOp>& op );

  /// RevBasicMemCtrl: register statistics
  void registerStats();

  /// RevBasicMemCtrl: inject statistics data for the target metric
  void recordStat( MemCtrlStats Stat, uint64_t Data = 1 );

  /// RevBasicMemCtrl: returns the total number of outstanding requests
  auto getTotalRqsts() const {
    return memOpNum[MemOp::MemOpREAD] + memOpNum[MemOp::MemOpWRITE] + memOpNum[MemOp::MemOpLOADLINK] +
           memOpNum[MemOp::MemOpREADLOCK] + memOpNum[MemOp::MemOpWRITEUNLOCK] + memOpNum[MemOp::MemOpCUSTOM];
  }

  // -- private data members
  StandardMem* memIface{};  ///< StandardMem memory interface
  bool         hasCache{};  ///< detects whether cache layers are present
  uint32_t     lineSize{};  ///< cache line size
  MemOpParams  memOpNum{};  ///< numbers in effect of memory parameters
  MemOpParams  memOpMax{};  ///< maximums allowable of memory parameters
  std::unordered_map<StandardMem::Request::id_t, std::shared_ptr<RevMemOp>> outstanding{};  ///< map of outstanding requests
  std::list<std::shared_ptr<RevMemOp>>                                      rqstQ{};        ///< queued memory requests

  ///< StandardMem interface response handlers
  const std::unique_ptr<RevStdMemHandlers> stdMemHandlers{ new RevStdMemHandlers( this, output.get() ) };

  /// RevBasicMemCtrl: map of amo operations to memory addresses
  std::unordered_multimap<uint64_t, std::tuple<uint32_t, void*, void*, RevFlag, std::shared_ptr<RevMemOp>, bool>> AMOTable{};

  std::vector<Statistic<uint64_t>*> stats{};  ///< statistics vector

};  // RevBasicMemCtrl

}  // namespace SST::RevCPU

#endif  // _SST_REVCPU_REVMEMCTRL_H_
