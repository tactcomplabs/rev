//
// _RevMemCtrl_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVMEMCTRL_H_
#define _SST_REVCPU_REVMEMCTRL_H_

// -- C++ Headers
#include <ctime>
#include <vector>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/output.h>
#include <sst/core/subcomponent.h>
#include <sst/core/interfaces/stdMem.h>

// -- RevCPU Headers
#include "RevOpts.h"

// RV{32,64} Register Operation Macros
                    //(r) = ((r) & (~r));
#define SEXT(r,x,b) do {\
                    (r) = ( (x) ^ ((1UL) << ((b) - 1)) ) - ((1UL) << ((b) - 1));\
                    }while(0)                // Sign extend the target register
#define ZEXT(r,x,b) do {\
                    (r) = (x) & (((1UL) << (b)) - 1);\
                    }while(0)                // Zero extend the target register

#define SEXTI(r,b)  do {\
                    (r) = ( (r) ^ ((1UL) << ((b) - 1)) ) - ((1UL) << ((b) - 1));\
                    }while(0)                // Sign extend the target register inline
#define ZEXTI(r,b)  do {\
                    (r) = (r) & (((1UL) << (b)) - 1);\
                    }while(0)                // Zero extend the target register inline

namespace SST::RevCPU {
  class RevMemCtrl;
}

using namespace SST::RevCPU;
using namespace SST::Interfaces;

namespace SST {
  namespace RevCPU {

    // ----------------------------------------
    // Extended StandardMem::Request::Flag enums
    // ----------------------------------------
    enum class RevFlag {
      F_NONCACHEABLE = 1 << 1,/// non cacheable
      F_SEXT32 = 1 << 17,     /// sign extend the 32bit result
      F_SEXT64 = 1 << 18,     /// sign extend the 64bit result
      F_ZEXT32 = 1 << 19,     /// zero extend the 32bit result
      F_ZEXT64 = 1 << 20      /// zero extend the 64bit result
    };

    // ----------------------------------------
    // RevMemOp
    // ----------------------------------------
    class RevMemOp {
    public:
      typedef enum{
        MemOpREAD           = 0,
        MemOpWRITE          = 1,
        MemOpFLUSH          = 2,
        MemOpREADLOCK       = 3,
        MemOpWRITEUNLOCK    = 4,
        MemOpLOADLINK       = 5,
        MemOpSTORECOND      = 6,
        MemOpCUSTOM         = 7,
        MemOpFENCE          = 8
      }MemOp;

      /// RevMemOp constructor
      RevMemOp( uint64_t Addr, uint64_t PAddr, uint32_t Size,
                RevMemOp::MemOp Op, StandardMem::Request::flags_t flags );

      /// RevMemOp constructor
      RevMemOp( uint64_t Addr, uint64_t PAddr, uint32_t Size, void *target,
                RevMemOp::MemOp Op, StandardMem::Request::flags_t flags );

      /// RevMemOp overloaded constructor
      RevMemOp( uint64_t Addr, uint64_t PAddr, uint32_t Size, char *buffer,
                RevMemOp::MemOp Op, StandardMem::Request::flags_t flags );

      /// RevMemOp overloaded constructor
      RevMemOp( uint64_t Addr, uint64_t PAddr, uint32_t Size, void *target,
                unsigned CustomOpc, RevMemOp::MemOp Op,
                StandardMem::Request::flags_t flags );

      /// RevMemOp overloaded constructor
      RevMemOp( uint64_t Addr, uint64_t PAddr, uint32_t Size, char *buffer,
                unsigned CustomOpc, RevMemOp::MemOp Op,
                StandardMem::Request::flags_t flags );

      /// RevMemOp destructor
      ~RevMemOp();

      /// RevMemOp: retrieve the memory operation type
      MemOp getOp() { return Op; }

      /// RevMemOp: retrieve the custom opcode
      unsigned getCustomOpc() { return CustomOpc; }

      /// RevMemOp: retrieve the target address
      uint64_t getAddr() { return Addr; }

      /// RevMemOp: retrieve the target physical address
      uint64_t getPhysAddr() { return PAddr; }

      /// RevMemOp: retrieve the size of the request
      uint32_t getSize() { return Size; }

      /// RevMemOp: retrieve the memory buffer
      std::vector<uint8_t> getBuf() { return membuf; }

      /// RevMemOp: retrieve the memory operation flags
      StandardMem::Request::flags_t getFlags() { return flags; }

      /// RevMemOp: retrieve the standard set of memory flags for MemEventBase
      StandardMem::Request::flags_t getStdFlags() { return ((uint32_t)(flags) & 0b1111111111111111); }

      /// RevMemOp: retrieve the flags for MemEventBase without caching enable
      StandardMem::Request::flags_t getNonCacheFlags() { return ((uint32_t)(flags) & 0b1111111111111101); }

      /// RevMemOp: sets the number of split cache line requests
      void setSplitRqst(unsigned S){ SplitRqst = S; }

      /// RevMemOp: set the invalidate flag
      void setInv(bool I){ Inv = I; }

      /// RevMemOp: retrieve the invalidate flag
      bool getInv() { return Inv; }

      /// RevMemOp: retrieve the number of split cache line requests
      unsigned getSplitRqst() { return SplitRqst; }

      /// RevMemOp: retrieve the target address
      void *getTarget() { return target; }

      // RevMemOp: determine if the request is cache-able
      bool isCacheable() { if( (flags & 0b10) > 0 ){ return false; } return true; }

    private:
      uint64_t Addr;      ///< RevMemOp: address
      uint64_t PAddr;     ///< RevMemOp: physical address (for RevMem I/O)
      uint32_t Size;      ///< RevMemOp: size of the memory operation in bytes
      bool Inv;           ///< RevMemOp: flush operation invalidate flag
      MemOp Op;           ///< RevMemOp: target memory operation
      unsigned CustomOpc; ///< RevMemOp: custom memory opcode
      unsigned SplitRqst; ///< RevMemOp: number of split cache line requests
      std::vector<uint8_t> membuf;          ///< RevMemOp: buffer
      StandardMem::Request::flags_t flags;  ///< RevMemOp: request flags
      void *target;                         ///< RevMemOp: target register pointer
    };

    // ----------------------------------------
    // RevMemCtrl
    // ----------------------------------------
    class RevMemCtrl : public SST::SubComponent {
    public:

      SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::RevMemCtrl)

      SST_ELI_DOCUMENT_PARAMS({ "verbose", "Set the verbosity of output for the memory controller", "0" }
      )

      /// RevMemCtrl: default constructor
      RevMemCtrl( ComponentId_t id, Params& params);

      /// RevMemCtrl: default destructor
      virtual ~RevMemCtrl();

      /// RevMemCtrl: initialization function
      virtual void init(unsigned int phase) = 0;

      /// RevMemCtrl: setup function
      virtual void setup() = 0;

      /// RevMemCtrl: finish function
      virtual void finish() = 0;

      /// RevMemCtrl: determines if outstanding requests exist
      virtual bool outstandingRqsts() = 0;

      /// RevMemCtrl: send flush request
      virtual bool sendFLUSHRequest(uint64_t Addr, uint64_t PAddr,
                                    uint32_t Size, bool Inv,
                                    StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a read request
      virtual bool sendREADRequest(uint64_t Addr, uint64_t PAddr,
                                   uint32_t Size, void *target,
                                   StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a write request
      virtual bool sendWRITERequest(uint64_t Addr, uint64_t PAddr,
                                    uint32_t Size, char *buffer,
                                    StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a readlock request
      virtual bool sendREADLOCKRequest(uint64_t Addr, uint64_t PAddr,
                                       uint32_t Size, void *target,
                                       StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a writelock request
      virtual bool sendWRITELOCKRequest(uint64_t Addr, uint64_t PAddr,
                                        uint32_t Size, char *buffer,
                                        StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a loadlink request
      virtual bool sendLOADLINKRequest(uint64_t Addr, uint64_t PAddr,
                                       uint32_t Size,
                                       StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a storecond request
      virtual bool sendSTORECONDRequest(uint64_t Addr, uint64_t PAddr,
                                        uint32_t Size, char *buffer,
                                        StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send an void custom read memory request
      virtual bool sendCUSTOMREADRequest(uint64_t Addr, uint64_t PAddr,
                                         uint32_t Size, void *target,
                                         unsigned Opc,
                                         StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a custom write request
      virtual bool sendCUSTOMWRITERequest(uint64_t Addr, uint64_t PAddr,
                                          uint32_t Size, char *buffer,
                                          unsigned Opc,
                                          StandardMem::Request::flags_t flags) = 0;

      /// RevMemCtrl: send a FENCE request
      virtual bool sendFENCE() = 0;

      /// RevMemCtrl: handle a read response
      virtual void handleReadResp(StandardMem::ReadResp* ev) = 0;

      /// RevMemCtrl: handle a write response
      virtual void handleWriteResp(StandardMem::WriteResp* ev) = 0;

      /// RevMemCtrl: handle a flush response
      virtual void handleFlushResp(StandardMem::FlushResp* ev) = 0;

      /// RevMemCtrl: handle a custom response
      virtual void handleCustomResp(StandardMem::CustomResp* ev) = 0;

      /// RevMemCtrl: handle an invalidate response
      virtual void handleInvResp(StandardMem::InvNotify* ev) = 0;

      /// RevMemCtrl: handle RevMemCtrl flags for write responses
      virtual void handleFlagResp(RevMemOp *op) = 0;

      /// RevMemCtrl: returns the cache line size
      virtual unsigned getLineSize() = 0;

    protected:
      SST::Output *output;        ///< RevMemCtrl: sst output object
    }; // class RevMemCtrl

    // ----------------------------------------
    // RevBasicMemCtrl
    // ----------------------------------------
    class RevBasicMemCtrl : public RevMemCtrl{
    public:
      SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(RevBasicMemCtrl, "revcpu",
                                            "RevBasicMemCtrl",
                                            SST_ELI_ELEMENT_VERSION(1, 0, 0),
                                            "RISC-V Rev basic memHierachy controller",
                                            SST::RevCPU::RevMemCtrl
                                           )

      SST_ELI_DOCUMENT_PARAMS({ "verbose",        "Set the verbosity of output for the memory controller",    "0" },
                              { "clock",          "Sets the clock frequency of the memory conroller",         "1Ghz" },
                              { "max_loads",      "Sets the maximum number of outstanding loads",             "64"},
                              { "max_stores",     "Sets the maximum number of outstanding stores",            "64"},
                              { "max_flush",      "Sets the maxmium number of oustanding flush events",       "64"},
                              { "max_llsc",       "Sets the maximum number of outstanding LL/SC events",      "64"},
                              { "max_readlock",   "Sets the maxmium number of outstanding readlock events",   "64"},
                              { "max_writeunlock","Sets the maximum number of outstanding writeunlock events","64"},
                              { "max_custom",     "Sets the maximum number of outstanding custom events",     "64"},
                              { "ops_per_cycle",  "Sets the maximum number of operations to issue per cycle", "2" }
      )

      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS({ "memIface", "Set the interface to memory", "SST::Interfaces::StandardMem" })

      SST_ELI_DOCUMENT_PORTS()

      SST_ELI_DOCUMENT_STATISTICS(
        {"ReadInFlight",        "Counts the number of reads in flight",             "count", 1},
        {"ReadPending",         "Counts the number of reads pending",               "count", 1},
        {"ReadBytes",           "Counts the number of bytes read",                  "bytes", 1},
        {"WriteInFlight",       "Counts the number of writes in flight",            "count", 1},
        {"WritePending",        "Counts the number of writes pending",              "count", 1},
        {"WriteBytes",          "Counts the number of bytes written",               "bytes", 1},
        {"FlushInFlight",       "Counts the number of flushes in flight",           "count", 1},
        {"FlushPending",        "Counts the number of flushes pending",             "count", 1},
        {"ReadLockInFlight",    "Counts the number of readlocks in flight",         "count", 1},
        {"ReadLockPending",     "Counts the number of readlocks pending",           "count", 1},
        {"ReadLockBytes",       "Counts the number of readlock bytes read",         "bytes", 1},
        {"WriteUnlockInFlight", "Counts the number of write unlocks in flight",     "count", 1},
        {"WriteUnlockPending",  "Counts the number of write unlocks pending",       "count", 1},
        {"WriteUnlockBytes",    "Counts the number of write unlock bytes written",  "bytes", 1},
        {"LoadLinkInFlight",    "Counts the number of loadlinks in flight",         "count", 1},
        {"LoadLinkPending",     "Counts the number of loadlinks pending",           "count", 1},
        {"StoreCondInFlight",   "Counts the number of storeconds in flight",        "count", 1},
        {"StoreCondPending",    "Counts the number of storeconds pending",          "count", 1},
        {"CustomInFlight",      "Counts the number of custom commands in flight",   "count", 1},
        {"CustomPending",       "Counts the number of custom commands pending",     "count", 1},
        {"CustomBytes",         "Counts the number of bytes in custom transactions","bytes", 1},
        {"FencePending",        "Counts the number of fence operations pending",    "count", 1}
      )

      typedef enum{
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
        FencePending        = 21
      }MemCtrlStats;

      /// RevBasicMemCtrl: constructor
      RevBasicMemCtrl(ComponentId_t id, Params& params);

      /// RevBasicMemCtrl: destructor
      virtual ~RevBasicMemCtrl();

      /// RevBasicMemCtrl: initialization function
      virtual void init(unsigned int phase) override;

      /// RevBasicMemCtrl: setup function
      virtual void setup() override;

      /// RevBasicMemCtrl: finish function
      virtual void finish() override;

      /// RevBasicMemCtrl: clock tick function
      virtual bool clockTick(Cycle_t cycle);

      /// RevBasicMemCtrl: determines if outstanding requests exist
      bool outstandingRqsts() override;

      /// RevBasicMemCtrl: returns the cache line size
      unsigned getLineSize() override { return lineSize; }

      /// RevBasicMemCtrl: memory event processing handler
      void processMemEvent(StandardMem::Request* ev);

      /// RevBasicMemCtrl: send a flush request
      virtual bool sendFLUSHRequest(uint64_t Addr, uint64_t PAdr, uint32_t Size,
                                    bool Inv,
                                    StandardMem::Request::flags_t flags) override;

      /// RevBasicMemCtrl: send a read request
      virtual bool sendREADRequest(uint64_t Addr, uint64_t PAddr,
                                   uint32_t Size, void *target,
                                   StandardMem::Request::flags_t flags) override;

      /// RevBasicMemCtrl: send a write request
      virtual bool sendWRITERequest(uint64_t Addr, uint64_t PAddr,
                                    uint32_t Size, char *buffer,
                                    StandardMem::Request::flags_t flags = 0) override;

      // RevBasicMemCtrl: send a readlock request
      virtual bool sendREADLOCKRequest(uint64_t Addr, uint64_t PAddr,
                                       uint32_t Size, void *target,
                                       StandardMem::Request::flags_t flags) override;

      // RevBasicMemCtrl: send a writelock request
      virtual bool sendWRITELOCKRequest(uint64_t Addr, uint64_t PAddr,
                                        uint32_t Size, char *buffer,
                                        StandardMem::Request::flags_t flags) override;

      // RevBasicMemCtrl: send a loadlink request
      virtual bool sendLOADLINKRequest(uint64_t Addr, uint64_t PAddr,
                                       uint32_t Size,
                                       StandardMem::Request::flags_t flags) override;

      // RevBasicMemCtrl: send a storecond request
      virtual bool sendSTORECONDRequest(uint64_t Addr, uint64_t PAddr,
                                        uint32_t Size, char *buffer,
                                        StandardMem::Request::flags_t flags) override;

      // RevBasicMemCtrl: send an void custom read memory request
      virtual bool sendCUSTOMREADRequest(uint64_t Addr, uint64_t PAddr,
                                         uint32_t Size, void *target,
                                         unsigned Opc,
                                         StandardMem::Request::flags_t flags) override;

      // RevBasicMemCtrl: send a custom write request
      virtual bool sendCUSTOMWRITERequest(uint64_t Addr, uint64_t PAddr,
                                          uint32_t Size, char *buffer,
                                          unsigned Opc,
                                          StandardMem::Request::flags_t flags) override;

      // RevBasicMemCtrl: send a FENCE request
      virtual bool sendFENCE() override;

      /// RevBasicMemCtrl: handle a read response
      virtual void handleReadResp(StandardMem::ReadResp* ev) override;

      /// RevBasicMemCtrl: handle a write response
      virtual void handleWriteResp(StandardMem::WriteResp* ev) override;

      /// RevBasicMemCtrl: handle a flush response
      virtual void handleFlushResp(StandardMem::FlushResp* ev) override;

      /// RevBasicMemCtrl: handle a custom response
      virtual void handleCustomResp(StandardMem::CustomResp* ev) override;

      /// RevBasicMemCtrl: handle an invalidate response
      virtual void handleInvResp(StandardMem::InvNotify* ev) override;

      /// RevBasicMemCtrl: handle RevMemCtrl flags for write responses
      virtual void handleFlagResp(RevMemOp *op) override;

    protected:
      // ----------------------------------------
      // RevStdMemHandlers
      // ----------------------------------------
      class RevStdMemHandlers : public Interfaces::StandardMem::RequestHandler {
      public:
        friend class RevBasicMemCtrl;

        /// RevStdMemHandlers: constructor
        RevStdMemHandlers( RevBasicMemCtrl* Ctrl, SST::Output* output);

        /// RevStdMemHandlers: destructor
        virtual ~RevStdMemHandlers();

        /// RevStdMemHandlers: handle read response
        virtual void handle(StandardMem::ReadResp* ev);

        /// RevStdMemhandlers: handle write response
        virtual void handle(StandardMem::WriteResp* ev);

        /// RevStdMemHandlers: handle flush response
        virtual void handle(StandardMem::FlushResp* ev);

        /// RevStdMemHandlers: handle custom response
        virtual void handle(StandardMem::CustomResp* ev);

        /// RevStdMemHandlers: handle invalidate response
        virtual void handle(StandardMem::InvNotify* ev);

      private:
        RevBasicMemCtrl *Ctrl;       ///< RevStdMemHandlers: memory controller object
      }; // class RevStdMemHandlers

    private:

      /// RevBasicMemCtrl: process the next memory request
      bool processNextRqst(unsigned &t_max_loads, unsigned &t_max_stores,
                           unsigned &t_max_flush, unsigned &t_max_llsc,
                           unsigned &t_max_readlock, unsigned &t_max_writeunlock,
                           unsigned &t_max_custom, unsigned &t_max_ops);

      /// RevBasicMemCtrl: determine if we can instantiate the target memory operation
      bool isMemOpAvail(RevMemOp *Op, unsigned &t_max_loads, unsigned &t_max_stores,
                        unsigned &t_max_flush, unsigned &t_max_llsc,
                        unsigned &t_max_readlock, unsigned &t_max_writeunlock,
                        unsigned &t_max_custom);

      /// RevBasicMemCtrl: build a standard memory request
      bool buildStandardMemRqst(RevMemOp *op, bool &Success);

      /// RevBasicMemCtrl: build raw memory requests with a 1-to-1 mapping to RevMemOps'
      bool buildRawMemRqst(RevMemOp *op, StandardMem::Request::flags_t TmpFlags);

      /// RevBasicMemCtrl: build cache-aligned requests
      bool buildCacheMemRqst(RevMemOp *op, bool &Success);

      /// RevBasicMemCtrl: register statistics
      void registerStats();

      /// RevBasicMemCtrl: inject statistics data for the target metric
      void recordStat(MemCtrlStats Stat, uint64_t Data);

      /// RevBasicMemCtrl: returns the total number of outstanding requests
      uint64_t getTotalRqsts();

      /// RevBasicMemCtrl: Determine the number of cache lines are required
      unsigned getNumCacheLines(uint64_t Addr, uint32_t Size);

      /// RevBasicMemCtrl: Retrieve the base cache line request size
      unsigned getBaseCacheLineSize(uint64_t Addr, uint32_t Size);

      /// RevBasicMemCtrl: retrieve the number of outstanding requests on the wire
      unsigned getNumSplitRqsts(RevMemOp *op);

      // -- private data members
      StandardMem* memIface;                  ///< StandardMem memory interface
      RevStdMemHandlers* stdMemHandlers;      ///< StandardMem interface response handlers
      bool hasCache;                          ///< detects whether cache layers are present
      unsigned lineSize;                      ///< cache line size
      unsigned max_loads;                     ///< maximum number of outstanding loads
      unsigned max_stores;                    ///< maximum number of outstanding stores
      unsigned max_flush;                     ///< maximum number of oustanding flush events
      unsigned max_llsc;                      ///< maximum number of outstanding llsc events
      unsigned max_readlock;                  ///< maximum number of oustanding readlock events
      unsigned max_writeunlock;               ///< maximum number of oustanding writelock events
      unsigned max_custom;                    ///< maximum number of oustanding custom events
      unsigned max_ops;                       ///< maximum number of ops to issue per cycle

      uint64_t num_read;                      ///< number of outstanding read requests
      uint64_t num_write;                     ///< number of outstanding write requests
      uint64_t num_flush;                     ///< number of outstanding flush requests
      uint64_t num_llsc;                      ///< number of outstanding LL/SC requests
      uint64_t num_readlock;                  ///< number of oustanding readlock requests
      uint64_t num_writeunlock;               ///< number of oustanding writelock requests
      uint64_t num_custom;                    ///< number of outstanding custom requests
      uint64_t num_fence;                     ///< number of oustanding fence requests

      std::vector<StandardMem::Request::id_t> requests;               ///< outstanding StandardMem requests
      std::vector<RevMemOp *> rqstQ;                                  ///< queued memory requests
      std::map<StandardMem::Request::id_t,RevMemOp *> outstanding;    ///< map of outstanding requests

      std::vector<Statistic<uint64_t>*> stats;                        ///< statistics vector

    }; // RevBasicMemCtrl
  } // namespace RevCPU
} // namespace SST

#endif // _SST_REVCPU_REVMEMCTRL_H_
