//
// _RevCPU_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_H_
#define _SST_REVCPU_H_

// -- Standard Headers
#include <vector>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/interfaces/simpleNetwork.h>

// -- Rev Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevLoader.h"
#include "RevProc.h"
#include "RevNIC.h"
#include "PanNet.h"

//using namespace SST::Interfaces;
//using namespace SST::RevCPU;

namespace SST {
  namespace RevCPU {
    class RevCPU : public SST::Component {

    public:
      /// RevCPU: top-level SST component constructor
      RevCPU( SST::ComponentId_t id, SST::Params& params );

      /// RevCPU: top-level SST component destructor
      ~RevCPU();

      /// RevCPU: standard SST component 'setup' function
      void setup();

      /// RevCPU: standard SST component 'finish' function
      void finish();

      /// RevCPU: standard SST component 'init' function
      void init( unsigned int phase );

      /// RevCPU: standard SST component clock function
      bool clockTick( SST::Cycle_t currentCycle );

      // -------------------------------------------------------
      // RevCPU Component Registration Data
      // -------------------------------------------------------
      /// RevCPU: Register the component with the SST core
      SST_ELI_REGISTER_COMPONENT(
                                  RevCPU,
                                  "revcpu",
                                  "RevCPU",
                                  SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                                  "RISC-V SST CPU",
                                  COMPONENT_CATEGORY_PROCESSOR
                                )

      // -------------------------------------------------------
      // RevCPU Component Parameter Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_PARAMS(
                              {"verbose",    "Sets the verbosity level of output",      "0" },
                              {"clock",      "Clock for the CPU",                       "1GHz" },
                              {"program",    "Sets the binary executable",              "a.out" },
                              {"args",       "Sets the argument list",                  ""},
                              {"numCores",   "Number of RISC-V cores to instantiate",   "1" },
                              {"memSize",    "Main memory size in bytes",               "1073741824"},
                              {"startAddr",  "Starting PC of the target core",          "core:0x80000000"},
                              {"machine",    "RISC-V machine model of the target core", "core:G"},
                              {"memCost",    "Memory latency range in cycles min:max",  "core:0:10"},
                              {"table",      "Instruction cost table",                  "core:/path/to/table"},
                              {"enable_nic", "Enable the internal RevNIC",              "0"},
                              {"enable_pan", "Enable PAN network endpoint",             "0"},
                              {"msgPerCycle","Number of messages per cycle to inject",  "1"},
                              {"splash",     "Display the splash logo",                 "0"}
                             )

      // -------------------------------------------------------
      // RevCPU Port Parameter Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_PORTS(
                            )

      // -------------------------------------------------------
      // RevCPU SubComponent Parameter Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
                                         {"nic", "Network interface", "SST::RevCPU::RevNIC"},
                                         {"pan_nic", "PAN Network interface", "SST::RevCPU::PanNet"}
                                         )

      // -------------------------------------------------------
      // RevCPU Component Statistics Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_STATISTICS(
                                 )

    private:
      unsigned numCores;                  ///< RevCPU: number of RISC-V cores
      unsigned msgPerCycle;               ///< RevCPU: number of messages to cycle to send
      std::string Exe;                    ///< RevCPU: binary executable
      std::string Args;                   ///< RevCPU: argument list
      RevOpts *Opts;                      ///< RevCPU: Simulation options object
      RevMem *Mem;                        ///< RevCPU: RISC-V main memory object
      RevLoader *Loader;                  ///< RevCPU: RISC-V loader
      std::vector<RevProc *> Procs;       ///< RevCPU: RISC-V processor objects
      bool *Enabled;                      ///< RevCPU: Completion structure

      uint8_t PrivTag;                    ///< RevCPU: private tag locator

      bool EnableNIC;                     ///< RevCPU: Flag for enabling the NIC
      bool EnablePAN;                     ///< RevCPU  flag for enabling the PAN operations

      TimeConverter* timeConverter;       ///< RevCPU: SST time conversion handler
      SST::Output output;                 ///< RevCPU: SST output handler

      nicAPI *Nic;                        ///< RevCPU: Network interface controller
      panNicAPI *PNic;                    ///< RevCPU: PAN network interface controller

      /// RevCPU: RevNIC message handler
      void handleMessage(SST::Event *ev);

      /// RevCPU: PAN NIC message handler
      void handlePANMessage(SST::Event *ev);

      /// RevCPU: Handle PAN host-side message (host only)
      void handleHostPANMessage(panNicEvent *event);

      /// RevCPU: Handle PAN network-side messages (NIC's or switches)
      void handleNetPANMessage(panNicEvent *event);

      /// RevCPU: Sends a PAN message
      bool sendPANMessage();

      /// RevCPU: handles the memory write operations from incoming PAN messages
      bool processPANMemRead();

      /// RevCPU: Creates a unique tag for this message
      uint8_t createTag();

      /// RevCPU: handle the SyncGet command
      void PANHandleSyncGet(panNicEvent *event);

      /// RevCPU: handle the SyncPut command
      void PANHandleSyncPut(panNicEvent *event);

      /// RevCPU: handle the AsyncGet command
      void PANHandleAsyncGet(panNicEvent *event);

      /// RevCPU: handle the AsyncPut command
      void PANHandleAsyncPut(panNicEvent *event);

      /// RevCPU: handle the SyncStreamGet commmand
      void PANHandleSyncStreamGet(panNicEvent *event);

      /// RevCPU: handle the SyncStreamPut command
      void PANHandleSyncStreamPut(panNicEvent *event);

      /// RevCPU: handle the AsyncStreamGet command
      void PANHandleAsyncStreamGet(panNicEvent *event);

      /// RevCPU: handle the AsyncStreamPut command
      void PANHandleAsyncStreamPut(panNicEvent *event);

      /// RevCPU: Exec command
      void PANHandleExec(panNicEvent *event);

      /// RevCPU: Status command
      void PANHandleStatus(panNicEvent *event);

      /// RevCPU: Cancel command
      void PANHandleCancel(panNicEvent *event);

      /// RevCPU: Reserve command
      void PANHandleReserve(panNicEvent *event);

      /// RevCPU: Revoke command
      void PANHandleRevoke(panNicEvent *event);

      /// RevCPU: Halt command
      void PANHandleHalt(panNicEvent *event);

      /// RevCPU: Resume command
      void PANHandleResume(panNicEvent *event);

      /// RevCPU: ReadReg command
      void PANHandleReadReg(panNicEvent *event);

      /// RevCPU: WriteReg command
      void PANHandleWriteReg(panNicEvent *event);

      /// RevCPU: SingleStep command
      void PANHandleSingleStep(panNicEvent *event);

      /// RevCPU: SetFuture command
      void PANHandleSetFuture(panNicEvent *event);

      /// RevCPU: RevokeFuture command
      void PANHandleRevokeFuture(panNicEvent *event);

      /// RevCPU: StatusFuture command
      void PANHandleStatusFuture(panNicEvent *event);

      /// RevCPU: BOTW command
      void PANHandleBOTW(panNicEvent *event);

      /// RevCPU: construct a failed PAN response command due to a token failure
      void PANBuildFailedToken(panNicEvent *event);

      /// RevCPU: construct a generic successful PAN response command
      void PANBuildSuccess(panNicEvent *event);

    }; // class RevCPU
  } // namespace RevCPU
} // namespace SST

#endif

// EOF
