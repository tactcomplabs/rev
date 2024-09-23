//
// _RevMemCtrl_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVMEMCTRL_H_
#define _SST_REVCPU_REVMEMCTRL_H_

// -- C++ Headers
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <tuple>
#include <type_traits>
#include <vector>

// -- SST Headers
#include "SST.h"

// -- RevCPU Headers
#include "RevCommon.h"
#include "RevOpts.h"
#include "RevTracer.h"

namespace SST::RevCPU {

using namespace SST::Interfaces;

// ----------------------------------------
// Extended StandardMem::Request::Flag enums
// ----------------------------------------
enum class RevFlag : uint32_t {
  F_NONE         = 0,         /// no special operation
  F_NONCACHEABLE = 1u << 1,   /// non cacheable
  F_BOXNAN       = 1u << 16,  /// NaN-box the 32-bit float
  F_SEXT32       = 1u << 17,  /// sign extend the 32bit result
  F_SEXT64       = 1u << 18,  /// sign extend the 64bit result
  F_ZEXT32       = 1u << 19,  /// zero extend the 32bit result
  F_ZEXT64       = 1u << 20,  /// zero extend the 64bit result
  F_AMOADD       = 1u << 21,  /// AMO Add
  F_AMOXOR       = 1u << 22,  /// AMO Xor
  F_AMOAND       = 1u << 23,  /// AMO And
  F_AMOOR        = 1u << 24,  /// AMO Or
  F_AMOMIN       = 1u << 25,  /// AMO Min
  F_AMOMAX       = 1u << 26,  /// AMO Max
  F_AMOMINU      = 1u << 27,  /// AMO Minu
  F_AMOMAXU      = 1u << 28,  /// AMO Maxu
  F_AMOSWAP      = 1u << 29,  /// AMO Swap
  F_AQ           = 1u << 30,  /// AMO AQ Flag
  F_RL           = 1u << 31,  /// AMO RL Flag
  F_ATOMIC       = F_AMOADD | F_AMOXOR | F_AMOAND | F_AMOOR | F_AMOMIN | F_AMOMAX | F_AMOMINU | F_AMOMAXU | F_AMOSWAP
};

// Ensure RevFlag is same underlying type as StandardMem::Request::flags_t
static_assert( std::is_same_v<StandardMem::Request::flags_t, std::underlying_type_t<RevFlag>> );

/// RevFlag: determine if the request is an AMO
constexpr bool RevFlagHas( RevFlag flag, RevFlag has ) {
  return ( static_cast<uint32_t>( flag ) & static_cast<uint32_t>( has ) ) != 0;
}

inline void RevFlagSet( RevFlag& flag, RevFlag set ) {
  flag = RevFlag{ uint32_t( flag ) | uint32_t( set ) };
}

/// RevFlag: Handle flag response
void RevHandleFlagResp( void* target, size_t size, RevFlag flags );

// ----------------------------------------
// RevMemOp
// ----------------------------------------
class RevMemOp {
public:
  /// RevMemOp constructor
  RevMemOp( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, MemOp Op, RevFlag flags );

  /// RevMemOp constructor
  RevMemOp( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, MemOp Op, RevFlag flags );

  /// RevMemOp overloaded constructor
  RevMemOp( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, MemOp Op, RevFlag flags );

  /// RevMemOp overloaded constructor
  RevMemOp( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, void* target, MemOp Op, RevFlag flags );

  /// RevMemOp overloaded constructor
  RevMemOp( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, std::vector<uint8_t> buffer, MemOp Op, RevFlag flags );

  /// RevMemOp overloaded constructor
  RevMemOp(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, unsigned CustomOpc, MemOp Op, RevFlag flags
  );

  /// RevMemOp overloaded constructor
  RevMemOp(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, unsigned CustomOpc, MemOp Op, RevFlag flags
  );

  /// RevMemOp default destructor
  ~RevMemOp()                            = default;

  /// Disallow copying and assignment
  RevMemOp( const RevMemOp& )            = delete;
  RevMemOp& operator=( const RevMemOp& ) = delete;

  /// RevMemOp: retrieve the memory operation type
  MemOp getOp() const { return Op; }

  /// RevMemOp: retrieve the custom opcode
  unsigned getCustomOpc() const { return CustomOpc; }

  /// RevMemOp: retrieve the target address
  uint64_t getAddr() const { return Addr; }

  /// RevMemOp: retrieve the target physical address
  uint64_t getPhysAddr() const { return PAddr; }

  /// RevMemOp: retrieve the size of the request
  uint32_t getSize() const { return Size; }

  /// RevMemOp: retrieve the memory buffer
  std::vector<uint8_t> getBuf() const { return membuf; }

  /// RevMemOp: retrieve the temporary target buffer
  std::vector<uint8_t> getTempT() const { return tempT; }

  /// RevMemOp: retrieve the memory operation flags
  RevFlag getFlags() const { return flags; }

  /// RevMemOp: retrieve the standard set of memory flags for MemEventBase
  RevFlag getStdFlags() const { return RevFlag{ static_cast<uint32_t>( flags ) & 0xFFFF }; }

  /// RevMemOp: retrieve the flags for MemEventBase without caching enable
  RevFlag getNonCacheFlags() const { return RevFlag{ static_cast<uint32_t>( flags ) & 0xFFFD }; }

  /// RevMemOp: sets the number of split cache line requests
  void setSplitRqst( unsigned S ) { SplitRqst = S; }

  /// RevMemOp: set the invalidate flag
  void setInv( bool I ) { Inv = I; }

  /// RevMemOp: set the hart
  void setHart( unsigned H ) { Hart = H; }

  /// RevMemOp: set the originating memory request
  void setMemReq( const MemReq& req ) { procReq = req; }

  /// RevMemOp: set the temporary target buffer
  void setTempT( std::vector<uint8_t> T );

  /// RevMemOp: retrieve the invalidate flag
  bool getInv() const { return Inv; }

  /// RevMemOp: retrieve the number of split cache line requests
  unsigned getSplitRqst() const { return SplitRqst; }

  /// RevMemOp: retrieve the target address
  void* getTarget() const { return target; }

  /// RevMemOp: retrieve the hart
  unsigned getHart() const { return Hart; }

  /// RevMemOp: Get the originating proc memory request
  const MemReq& getMemReq() const { return procReq; }

  // RevMemOp: determine if the request is cache-able
  bool isCacheable() const { return ( static_cast<uint32_t>( flags ) & static_cast<uint32_t>( RevFlag::F_NONCACHEABLE ) ) == 0; }

private:
  unsigned             Hart{};       ///< RevMemOp: RISC-V Hart
  uint64_t             Addr{};       ///< RevMemOp: address
  uint64_t             PAddr{};      ///< RevMemOp: physical address (for RevMem I/O)
  uint32_t             Size{};       ///< RevMemOp: size of the memory operation in bytes
  bool                 Inv{};        ///< RevMemOp: flush operation invalidate flag
  MemOp                Op{};         ///< RevMemOp: target memory operation
  unsigned             CustomOpc{};  ///< RevMemOp: custom memory opcode
  unsigned             SplitRqst{};  ///< RevMemOp: number of split cache line requests
  std::vector<uint8_t> membuf{};     ///< RevMemOp: buffer
  std::vector<uint8_t> tempT{};      ///< RevMemOp: temporary target buffer for R-M-W ops
  RevFlag              flags{};      ///< RevMemOp: request flags
  void*                target{};     ///< RevMemOp: target register pointer
  MemReq               procReq{};    ///< RevMemOp: original request from RevCore
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
  virtual ~RevMemCtrl();

  /// RevMemCtrl: disallow copying and assignment
  RevMemCtrl( const RevMemCtrl& )                                                                                       = delete;
  RevMemCtrl& operator=( const RevMemCtrl& )                                                                            = delete;

  /// RevMemCtrl: initialization function
  virtual void init( unsigned int phase )                                                                               = 0;

  /// RevMemCtrl: setup function
  virtual void setup()                                                                                                  = 0;

  /// RevMemCtrl: finish function
  virtual void finish()                                                                                                 = 0;

  /// RevMemCtrl: determines if outstanding requests exist
  virtual bool outstandingRqsts()                                                                                       = 0;

  /// RevMemCtrl: send flush request
  virtual bool sendFLUSHRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, bool Inv, RevFlag flags ) = 0;

  /// RevMemCtrl: send a read request
  virtual bool sendREADRequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  )                                                                                                                         = 0;

  /// RevMemCtrl: send a write request
  virtual bool sendWRITERequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, RevFlag flags ) = 0;

  /// RevMemCtrl: send an AMO request
  virtual bool sendAMORequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, void* target, const MemReq& req, RevFlag flags
  ) = 0;

  /// RevMemCtrl: send a readlock request
  virtual bool sendREADLOCKRequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  )                                                                                                                             = 0;

  /// RevMemCtrl: send a writelock request
  virtual bool sendWRITELOCKRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, RevFlag flags ) = 0;

  /// RevMemCtrl: send a loadlink request
  virtual bool sendLOADLINKRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags )                = 0;

  /// RevMemCtrl: send a storecond request
  virtual bool sendSTORECONDRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, RevFlag flags ) = 0;

  /// RevMemCtrl: send an void custom read memory request
  virtual bool sendCUSTOMREADRequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, unsigned Opc, RevFlag flags
  ) = 0;

  /// RevMemCtrl: send a custom write request
  virtual bool sendCUSTOMWRITERequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, unsigned Opc, RevFlag flags
  )                                                            = 0;

  /// RevMemCtrl: send a FENCE request
  virtual bool sendFENCE( unsigned Hart )                      = 0;

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

  /// RevMemCtrl: handle RevMemCtrl flags for write responses
  virtual void handleFlagResp( RevMemOp* op )                  = 0;

  /// RevMemCtrl: handle an AMO for the target READ+MODIFY+WRITE triplet
  virtual void handleAMO( RevMemOp* op )                       = 0;

  /// RevMemCtrl: returns the cache line size
  virtual unsigned getLineSize()                               = 0;

  /// Assign processor tracer
  virtual void setTracer( RevTracer* tracer )                  = 0;

protected:
  SST::Output* output{};  ///< RevMemCtrl: sst output object
  RevTracer*   Tracer{};  ///< RevMemCtrl: tracer pointer

};  // class RevMemCtrl

// ----------------------------------------
// RevBasicMemCtrl
// ----------------------------------------
class RevBasicMemCtrl : public RevMemCtrl {
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

  enum MemCtrlStats : uint32_t {
    ReadInFlight        = 0,
    ReadPending         = 1,
    ReadBytes           = 2,
    WriteInFlight       = 3,
    WritePending        = 4,
    WriteBytes          = 5,
    FlushInFlight       = 6,
    FlushPending        = 7,
    ReadLockInFlight    = 8,
    ReadLockPending     = 9,
    ReadLockBytes       = 10,
    WriteUnlockInFlight = 11,
    WriteUnlockPending  = 12,
    WriteUnlockBytes    = 13,
    LoadLinkInFlight    = 14,
    LoadLinkPending     = 15,
    StoreCondInFlight   = 16,
    StoreCondPending    = 17,
    CustomInFlight      = 18,
    CustomPending       = 19,
    CustomBytes         = 20,
    FencePending        = 21,
    AMOAddBytes         = 22,
    AMOAddPending       = 23,
    AMOXorBytes         = 24,
    AMOXorPending       = 25,
    AMOAndBytes         = 26,
    AMOAndPending       = 27,
    AMOOrBytes          = 28,
    AMOOrPending        = 29,
    AMOMinBytes         = 30,
    AMOMinPending       = 31,
    AMOMaxBytes         = 32,
    AMOMaxPending       = 33,
    AMOMinuBytes        = 34,
    AMOMinuPending      = 35,
    AMOMaxuBytes        = 36,
    AMOMaxuPending      = 37,
    AMOSwapBytes        = 38,
    AMOSwapPending      = 39,
  };

  /// RevBasicMemCtrl: constructor
  RevBasicMemCtrl( ComponentId_t id, const Params& params );

  /// RevBasicMemCtrl: destructor
  virtual ~RevBasicMemCtrl();

  /// RevBasicMemCtrl: disallow copying and assignment
  RevBasicMemCtrl( const RevBasicMemCtrl& )            = delete;
  RevBasicMemCtrl& operator=( const RevBasicMemCtrl& ) = delete;

  /// RevBasicMemCtrl: initialization function
  virtual void init( unsigned int phase ) override;

  /// RevBasicMemCtrl: setup function
  virtual void setup() override;

  /// RevBasicMemCtrl: finish function
  virtual void finish() override;

  /// RevBasicMemCtrl: clock tick function
  virtual bool clockTick( Cycle_t cycle );

  /// RevBasicMemCtrl: determines if outstanding requests exist
  bool outstandingRqsts() override;

  /// RevBasicMemCtrl: returns the cache line size
  unsigned getLineSize() override { return lineSize; }

  /// RevBasicMemCtrl: memory event processing handler
  void processMemEvent( StandardMem::Request* ev );

  /// RevBasicMemCtrl: send a flush request
  virtual bool sendFLUSHRequest( unsigned Hart, uint64_t Addr, uint64_t PAdr, uint32_t Size, bool Inv, RevFlag flags ) override;

  /// RevBasicMemCtrl: send a read request
  virtual bool sendREADRequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  ) override;

  /// RevBasicMemCtrl: send a write request
  virtual bool sendWRITERequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, RevFlag flags = RevFlag::F_NONE
  ) override;

  /// RevBasicMemCtrl: send an AMO request
  virtual bool sendAMORequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, void* target, const MemReq& req, RevFlag flags
  ) override;

  // RevBasicMemCtrl: send a readlock request
  virtual bool sendREADLOCKRequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, const MemReq& req, RevFlag flags
  ) override;

  // RevBasicMemCtrl: send a writelock request
  virtual bool
    sendWRITELOCKRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, RevFlag flags ) override;

  // RevBasicMemCtrl: send a loadlink request
  virtual bool sendLOADLINKRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, RevFlag flags ) override;

  // RevBasicMemCtrl: send a storecond request
  virtual bool
    sendSTORECONDRequest( unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, RevFlag flags ) override;

  // RevBasicMemCtrl: send an void custom read memory request
  virtual bool sendCUSTOMREADRequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, void* target, unsigned Opc, RevFlag flags
  ) override;

  // RevBasicMemCtrl: send a custom write request
  virtual bool sendCUSTOMWRITERequest(
    unsigned Hart, uint64_t Addr, uint64_t PAddr, uint32_t Size, char* buffer, unsigned Opc, RevFlag flags
  ) override;

  // RevBasicMemCtrl: send a FENCE request
  virtual bool sendFENCE( unsigned Hart ) override;

  /// RevBasicMemCtrl: handle a read response
  virtual void handleReadResp( StandardMem::ReadResp* ev ) override;

  /// RevBasicMemCtrl: handle a write response
  virtual void handleWriteResp( StandardMem::WriteResp* ev ) override;

  /// RevBasicMemCtrl: handle a flush response
  virtual void handleFlushResp( StandardMem::FlushResp* ev ) override;

  /// RevBasicMemCtrl: handle a custom response
  virtual void handleCustomResp( StandardMem::CustomResp* ev ) override;

  /// RevBasicMemCtrl: handle an invalidate response
  virtual void handleInvResp( StandardMem::InvNotify* ev ) override;

  /// RevBasicMemCtrl: handle RevMemCtrl flags for write responses
  virtual void handleFlagResp( RevMemOp* op ) override { RevHandleFlagResp( op->getTarget(), op->getSize(), op->getFlags() ); }

  /// RevBasicMemCtrl: handle an AMO for the target READ+MODIFY+WRITE triplet
  virtual void handleAMO( RevMemOp* op ) override;

  /// RevBasicMemCtrl: assign tracer pointer
  virtual void setTracer( RevTracer* tracer ) override;

protected:
  // ----------------------------------------
  // RevStdMemHandlers
  // ----------------------------------------
  class RevStdMemHandlers : public Interfaces::StandardMem::RequestHandler {
  public:
    friend class RevBasicMemCtrl;

    /// RevStdMemHandlers: constructor
    RevStdMemHandlers( RevBasicMemCtrl* Ctrl, SST::Output* output );

    /// RevStdMemHandlers: destructor
    virtual ~RevStdMemHandlers();

    /// RevStdMemHandlers: disallow copying and assignment
    RevStdMemHandlers( const RevStdMemHandlers& )            = delete;
    RevStdMemHandlers& operator=( const RevStdMemHandlers& ) = delete;

    /// RevStdMemHandlers: handle read response
    virtual void handle( StandardMem::ReadResp* ev );

    /// RevStdMemhandlers: handle write response
    virtual void handle( StandardMem::WriteResp* ev );

    /// RevStdMemHandlers: handle flush response
    virtual void handle( StandardMem::FlushResp* ev );

    /// RevStdMemHandlers: handle custom response
    virtual void handle( StandardMem::CustomResp* ev );

    /// RevStdMemHandlers: handle invalidate response
    virtual void handle( StandardMem::InvNotify* ev );

  private:
    RevBasicMemCtrl* Ctrl{};  ///< RevStdMemHandlers: memory controller object

  };  // class RevStdMemHandlers

private:
  /// RevBasicMemCtrl: process the next memory request
  bool processNextRqst(
    unsigned& t_max_loads,
    unsigned& t_max_stores,
    unsigned& t_max_flush,
    unsigned& t_max_llsc,
    unsigned& t_max_readlock,
    unsigned& t_max_writeunlock,
    unsigned& t_max_custom,
    unsigned& t_max_ops
  );

  /// RevBasicMemCtrl: determine if we can instantiate the target memory operation
  bool isMemOpAvail(
    RevMemOp* Op,
    unsigned& t_max_loads,
    unsigned& t_max_stores,
    unsigned& t_max_flush,
    unsigned& t_max_llsc,
    unsigned& t_max_readlock,
    unsigned& t_max_writeunlock,
    unsigned& t_max_custom
  );

  /// RevBasicMemCtrl: build a standard memory request
  bool buildStandardMemRqst( RevMemOp* op, bool& Success );

  /// RevBasicMemCtrl: build raw memory requests with a 1-to-1 mapping to RevMemOps'
  bool buildRawMemRqst( RevMemOp* op, RevFlag TmpFlags );

  /// RevBasicMemCtrl: build cache-aligned requests
  bool buildCacheMemRqst( RevMemOp* op, bool& Success );

  /// RevBasicMemCtrl: determine if there are any pending AMOs that would prevent a request from dispatching
  bool isPendingAMO( unsigned Slot );

  /// RevBasicMemCtrl: determine if we need to utilize AQ ordering semantics
  bool isAQ( unsigned Slot, unsigned Hart );

  /// RevBasicMemCtrl: determine if we need to utilize RL ordering semantics
  bool isRL( unsigned Slot, unsigned Hart );

  /// RevBasicMemCtrl: register statistics
  void registerStats();

  /// RevBasicMemCtrl: inject statistics data for the target metric
  void recordStat( MemCtrlStats Stat, uint64_t Data );

  /// RevBasicMemCtrl: returns the total number of outstanding requests
  uint64_t getTotalRqsts();

  /// RevBasicMemCtrl: Determine the number of cache lines are required
  unsigned getNumCacheLines( uint64_t Addr, uint32_t Size );

  /// RevBasicMemCtrl: Retrieve the base cache line request size
  unsigned getBaseCacheLineSize( uint64_t Addr, uint32_t Size );

  /// RevBasicMemCtrl: retrieve the number of outstanding requests on the wire
  unsigned getNumSplitRqsts( RevMemOp* op );

  /// RevBasicMemCtrl: perform the MODIFY portion of the AMO (READ+MODIFY+WRITE)
  void performAMO( std::tuple<unsigned, char*, void*, RevFlag, RevMemOp*, bool> Entry );

  // -- private data members
  StandardMem*       memIface{};         ///< StandardMem memory interface
  RevStdMemHandlers* stdMemHandlers{};   ///< StandardMem interface response handlers
  bool               hasCache{};         ///< detects whether cache layers are present
  unsigned           lineSize{};         ///< cache line size
  unsigned           max_loads{};        ///< maximum number of outstanding loads
  unsigned           max_stores{};       ///< maximum number of outstanding stores
  unsigned           max_flush{};        ///< maximum number of oustanding flush events
  unsigned           max_llsc{};         ///< maximum number of outstanding llsc events
  unsigned           max_readlock{};     ///< maximum number of oustanding readlock events
  unsigned           max_writeunlock{};  ///< maximum number of oustanding writelock events
  unsigned           max_custom{};       ///< maximum number of oustanding custom events
  unsigned           max_ops{};          ///< maximum number of ops to issue per cycle

  uint64_t num_read{};         ///< number of outstanding read requests
  uint64_t num_write{};        ///< number of outstanding write requests
  uint64_t num_flush{};        ///< number of outstanding flush requests
  uint64_t num_llsc{};         ///< number of outstanding LL/SC requests
  uint64_t num_readlock{};     ///< number of oustanding readlock requests
  uint64_t num_writeunlock{};  ///< number of oustanding writelock requests
  uint64_t num_custom{};       ///< number of outstanding custom requests
  uint64_t num_fence{};        ///< number of oustanding fence requests

  std::vector<StandardMem::Request::id_t>         requests{};     ///< outstanding StandardMem requests
  std::vector<RevMemOp*>                          rqstQ{};        ///< queued memory requests
  std::map<StandardMem::Request::id_t, RevMemOp*> outstanding{};  ///< map of outstanding requests

#define AMOTABLE_HART   0
#define AMOTABLE_BUFFER 1
#define AMOTABLE_TARGET 2
#define AMOTABLE_FLAGS  3
#define AMOTABLE_MEMOP  4
#define AMOTABLE_IN     5

  /// RevBasicMemCtrl: map of amo operations to memory addresses
  std::multimap<uint64_t, std::tuple<unsigned, char*, void*, RevFlag, RevMemOp*, bool>> AMOTable{};

  std::vector<Statistic<uint64_t>*> stats{};  ///< statistics vector

};  // RevBasicMemCtrl

///< Apply Atomic Memory Operation
/// The operation described by "flags" is applied to memory "Target" with value "value"
template<typename T>
void ApplyAMO( RevFlag flags, void* Target, T value ) {
  // Target and value cast to signed and unsigned versions
  auto* TmpTarget  = static_cast<std::make_signed_t<T>*>( Target );
  auto* TmpTargetU = static_cast<std::make_unsigned_t<T>*>( Target );
  auto  TmpBuf     = static_cast<std::make_signed_t<T>>( value );
  auto  TmpBufU    = static_cast<std::make_unsigned_t<T>>( value );

  // Table mapping atomic operations to executable code
  // clang-format off
  static const std::pair<RevCPU::RevFlag, std::function<void()>> table[] = {
    { RevFlag::F_AMOADD,  [&]{ *TmpTarget += TmpBuf; } },
    { RevFlag::F_AMOXOR,  [&]{ *TmpTarget ^= TmpBuf; } },
    { RevFlag::F_AMOAND,  [&]{ *TmpTarget &= TmpBuf; } },
    { RevFlag::F_AMOOR,   [&]{ *TmpTarget |= TmpBuf; } },
    { RevFlag::F_AMOSWAP, [&]{ *TmpTarget  = TmpBuf; } },
    { RevFlag::F_AMOMIN,  [&]{ *TmpTarget  = std::min(*TmpTarget,  TmpBuf);  } },
    { RevFlag::F_AMOMAX,  [&]{ *TmpTarget  = std::max(*TmpTarget,  TmpBuf);  } },
    { RevFlag::F_AMOMINU, [&]{ *TmpTargetU = std::min(*TmpTargetU, TmpBufU); } },
    { RevFlag::F_AMOMAXU, [&]{ *TmpTargetU = std::max(*TmpTargetU, TmpBufU); } },
  };
  // clang-format on
  for( auto& [flag, op] : table ) {
    if( RevFlagHas( flags, flag ) ) {
      op();
      break;
    }
  }
}

}  // namespace SST::RevCPU

#endif  // _SST_REVCPU_REVMEMCTRL_H_
