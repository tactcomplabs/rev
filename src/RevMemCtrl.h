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

      SST_ELI_DOCUMENT_PARAMS({ "verbose", "Set the verbosity of output for the memory controller", "0" },
                              { "clock",   "Sets the clock frequency of the memory conroller", "1Ghz" }
      )

      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS({ "memIface", "Set the interface to memory", "SST::Interfaces::StandardMem" })

      SST_ELI_DOCUMENT_PORTS({ "cache_link", "Connects the controller to the first level of cache/memory", {} })

      SST_ELI_DOCUMENT_STATISTICS()

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
      StandardMem* memIface;                  ///< StandardMem memory interface
      RevStdMemHandlers* stdMemHandlers;      ///< StandardMem interface response handlers

    }; // RevBasicMemCtrl
  } // namespace RevCPU
} // namespace SST

#endif // _SST_REVCPU_REVMEMCTRL_H_
