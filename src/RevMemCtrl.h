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

namespace SST::RevCPU {
  class RevMemCtrl;
}

using namespace SST::RevCPU;
using namespace SST::Interfaces;

namespace SST {
  namespace RevCPU {

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

    protected:
      SST::Output *output;       ///< RevMemCtrl: sst output object

    }; // class RevMemCtrl

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
                              { "ops_per_cycle",  "Sets the maximum number of operations to issue per cycle", "2" }
      )

      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS({ "memIface", "Set the interface to memory", "SST::Interfaces::StandardMem" })

      SST_ELI_DOCUMENT_PORTS({ "cache_link", "Connects the controller to the first level of cache/memory", {} })

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
        {"StoreCondPending",    "Counts the number of storeconds pending",          "count", 1}
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
        StoreCondPending    = 17
      }MemCtrlStats;

      /// RevBasicMemCtrl: constructor
      RevBasicMemCtrl(ComponentId_t id, Params& params);

      /// RevBasicMemCtrl: destructor
      virtual ~RevBasicMemCtrl();

      /// RevBasicMemCtrl: initialization function
      virtual void init(unsigned int phase) override;

      /// RevBasicMemCtrl: clock tick function
      virtual bool clockTick(Cycle_t cycle);

      /// RevBasicMemCtrl: memory event processing handler
      void processMemEvent(StandardMem::Request* ev);

    protected:
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

      private:
        RevBasicMemCtrl *ctrl;       ///< RevStdMemHandlers: memory controller object
      }; // class RevStdMemHandlers


    private:

      /// RevBasicMemCtrl: register statistics
      void registerStats();

      /// RevBasicMemCtrl: inject statistics data for the target metric
      void recordStat(MemCtrlStats Stat, uint64_t Data);

      // -- private data members
      StandardMem* memIface;                  ///< StandardMem memory interface
      RevStdMemHandlers* stdMemHandlers;      ///< StandardMem interface response handlers
      unsigned max_loads;                     ///< maximum number of outstanding loads
      unsigned max_stores;                    ///< maximum number of outstanding stores
      unsigned max_ops;                       ///< maximum number of ops to issue per cycle

      std::vector<Statistic<uint64_t>*> stats;///< statistics vector

    }; // RevBasicMemCtrl
  } // namespace RevCPU
} // namespace SST

#endif // _SST_REVCPU_REVMEMCTRL_H_
