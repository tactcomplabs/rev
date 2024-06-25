//
// _RevCPU_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_H_
#define _SST_REVCPU_H_

// -- Standard Headers
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <tuple>
#include <utility>
#include <vector>

// -- SST Headers
#include "SST.h"

// -- Rev Headers
#include "RevCoProc.h"
#include "RevCore.h"
#include "RevLoader.h"
#include "RevMem.h"
#include "RevMemCtrl.h"
#include "RevNIC.h"
#include "RevOpts.h"
#include "RevRand.h"
#include "RevThread.h"

namespace SST::RevCPU {

class RevCPU : public SST::Component {

public:
  /// RevCPU: top-level SST component constructor
  RevCPU( SST::ComponentId_t id, const SST::Params& params );

  /// RevCPU: top-level SST component destructor
  ~RevCPU();

  /// RevCPU: disallow copying and assignment
  RevCPU( const RevCPU& )            = delete;
  RevCPU& operator=( const RevCPU& ) = delete;

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
    RevCPU,    // component class
    "revcpu",  // component library
    "RevCPU",  // component name
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "RISC-V SST CPU",
    COMPONENT_CATEGORY_PROCESSOR
  )

  // -------------------------------------------------------
  // RevCPU Component Parameter Data
  // -------------------------------------------------------
  // clang-format off
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",           "0" },
    {"clock",           "Clock for the CPU",                            "1GHz" },
    {"program",         "Sets the binary executable",                   "a.out" },
    {"args",            "Sets the argument list",                       ""},
    {"numCores",        "Number of RISC-V cores to instantiate",        "1" },
    {"numHarts",        "Number of harts (per core) to instantiate",    "1" },
    {"memSize",         "Main memory size in bytes",                    "1073741824"},
    {"startAddr",       "Starting PC of the target core",               "core:0x80000000"},
    {"startSymbol",     "Starting symbol name of the target core",      "core:symbol"},
    {"machine",         "RISC-V machine model of the target core",      "core:G"},
    {"memCost",         "Memory latency range in cycles min:max",       "core:0:10"},
    {"prefetchDepth",   "Instruction prefetch depth per core",          "core:1"},
    {"table",           "Instruction cost table",                       "core:/path/to/table"},
    {"enable_nic",      "Enable the internal RevNIC",                   "0"},
    {"enable_pan",      "Enable PAN network endpoint",                  "0"},
    {"enable_test",     "Enable PAN network endpoint test",             "0"},
    {"enable_pan_stats", "Enable PAN network statistics",               "1"},
    {"enable_memH",     "Enable memHierarchy",                          "0"},
    {"enableRDMAMbox",  "Enable the RDMA mailbox",                      "1"},
    {"enableCoProc",    "Enable an attached coProcessor for all cores", "0"},
    {"enable_faults",   "Enable the fault injection logic",             "0"},
    {"faults",          "Enable specific faults",                       "decode,mem,reg,alu"},
    {"fault_width",     "Specify the bit width of potential faults",    "single,word,N"},
    {"fault_range",     "Specify the range of faults in cycles",        "65536"},
    {"msgPerCycle",     "Number of messages per cycle to inject",       "1"},
    {"RDMAPerCycle",    "Number of RDMA messages per cycle to inject",  "1"},
    {"testIters",       "Number of PAN test messages to send",          "255"},
    {"trcOp",           "Tracer instruction trigger",                   "slli"},
    {"trcLimit",        "Max trace lines per core (0 no limit)",        "0"},
    {"trcStartCycle",   "Starting tracer cycle (disables trcOp)",       "0"},
    {"splash",          "Display the splash logo",                      "0"},
    {"independentCoprocClock",  "Enables each coprocessor to register its own clock handler", "0"},
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
    {"pan_nic", "PAN Network interface", "SST::RevCPU::PanNet"},
    {"memory", "Memory interface to utilize for cache/memory hierachy", "SST::RevCPU::RevMemCtrl"},
    {"co_proc", "Co-processor attached to RevCore", "SST::RevCPU::RevSimpleCoProc"},
    )

  // -------------------------------------------------------
  // RevCPU Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS(
    {"SyncGetSend",         "Operation count for outgoing SyncGet Commands",        "count",  1},
    {"SyncPutSend",         "Operation count for outgoing SyncPut Commands",        "count",  1},
    {"AsyncGetSend",        "Operation count for outgoing AsyncGet Commands",       "count",  1},
    {"AsyncPutSend",        "Operation count for outgoing AsyncPut Commands",       "count",  1},
    {"SyncStreamGetSend",   "Operation count for outgoing SyncStreamGet Commands",  "count",  1},
    {"SyncStreamPutSend",   "Operation count for outgoing SyncStreamPut Commands",  "count",  1},
    {"AsyncStreamGetSend",  "Operation count for outgoing AsyncStreamGet Commands", "count",  1},
    {"AsyncStreamPutSend",  "Operation count for outgoing AsyncStreamPut Commands", "count",  1},
    {"ExecSend",            "Operation count for outgoing Exec Commands",           "count",  1},
    {"StatusSend",          "Operation count for outgoing Status Commands",         "count",  1},
    {"CancelSend",          "Operation count for outgoing Cancel Commands",         "count",  1},
    {"ReserveSend",         "Operation count for outgoing Reserve Commands",        "count",  1},
    {"RevokeSend",          "Operation count for outgoing Revoke Commands",         "count",  1},
    {"HaltSend",            "Operation count for outgoing Halt Commands",           "count",  1},
    {"ResumeSend",          "Operation count for outgoing Resume Commands",         "count",  1},
    {"ReadRegSend",         "Operation count for outgoing ReadReg Commands",        "count",  1},
    {"WriteRegSend",        "Operation count for outgoing WriteReg Commands",       "count",  1},
    {"SingleStepSend",      "Operation count for outgoing SingleStep Commands",     "count",  1},
    {"SetFutureSend",       "Operation count for outgoing SetFuture Commands",      "count",  1},
    {"RevokeFutureSend",    "Operation count for outgoing RevokeFuture Commands",   "count",  1},
    {"StatusFutureSend",    "Operation count for outgoing StatusFuture Commands",   "count",  1},
    {"SuccessSend",         "Operation count for outgoing Success Commands",        "count",  1},
    {"FailedSend",          "Operation count for outgoing Failed Commands",         "count",  1},
    {"BOTWSend",            "Operation count for outgoing BOTW Commands",           "count",  1},
    {"SyncGetRecv",         "Operation count for incoming SyncGet Commands",        "count",  1},
    {"SyncPutRecv",         "Operation count for incoming SyncPut Commands",        "count",  1},
    {"AsyncGetRecv",        "Operation count for incoming AsyncGet Commands",       "count",  1},
    {"AsyncPutRecv",        "Operation count for incoming AsyncPut Commands",       "count",  1},
    {"SyncStreamGetRecv",   "Operation count for incoming SyncStreamGet Commands",  "count",  1},
    {"SyncStreamPutRecv",   "Operation count for incoming SyncStreamPut Commands",  "count",  1},
    {"AsyncStreamGetRecv",  "Operation count for incoming AsyncStreamGet Commands", "count",  1},
    {"AsyncStreamPutRecv",  "Operation count for incoming AsyncStreamPut Commands", "count",  1},
    {"ExecRecv",            "Operation count for incoming Exec Commands",           "count",  1},
    {"StatusRecv",          "Operation count for incoming Status Commands",         "count",  1},
    {"CancelRecv",          "Operation count for incoming Cancel Commands",         "count",  1},
    {"ReserveRecv",         "Operation count for incoming Reserve Commands",        "count",  1},
    {"RevokeRecv",          "Operation count for incoming Revoke Commands",         "count",  1},
    {"HaltRecv",            "Operation count for incoming Halt Commands",           "count",  1},
    {"ResumeRecv",          "Operation count for incoming Resume Commands",         "count",  1},
    {"ReadRegRecv",         "Operation count for incoming ReadReg Commands",        "count",  1},
    {"WriteRegRecv",        "Operation count for incoming WriteReg Commands",       "count",  1},
    {"SingleStepRecv",      "Operation count for incoming SingleStep Commands",     "count",  1},
    {"SetFutureRecv",       "Operation count for incoming SetFuture Commands",      "count",  1},
    {"RevokeFutureRecv",    "Operation count for incoming RevokeFuture Commands",   "count",  1},
    {"StatusFutureRecv",    "Operation count for incoming StatusFuture Commands",   "count",  1},
    {"SuccessRecv",         "Operation count for incoming Success Commands",        "count",  1},
    {"FailedRecv",          "Operation count for incoming Failed Commands",         "count",  1},
    {"BOTWRecv",            "Operation count for incoming BOTW Commands",           "count",  1},

    {"CyclesWithIssue",     "Cycles with succesful instruction issue",              "count",  1},
    {"TotalCycles",         "Total clock cycles",                                   "count",  1},
    {"FloatsRead",          "Total SP floating point values read",                  "count",  1},
    {"FloatsWritten",       "Total SP floating point values written",               "count",  1},
    {"DoublesRead",         "Total DP floating point values read",                  "count",  1},
    {"DoublesWritten",      "Total DP floating point values written",               "count",  1},
    {"BytesRead",           "Total bytes read",                                     "count",  1},
    {"BytesWritten",        "Total bytes written",                                  "count",  1},
    {"FloatsExec",          "Total SP or DP float instructions executed",           "count",  1},
    {"TLBHitsPerCore",      "TLB hits per core",                                    "count",  1},
    {"TLBMissesPerCore",    "TLB misses per core",                                  "count",  1},

    {"TLBHits",             "TLB hits",                                             "count",  1},
    {"TLBMisses",           "TLB misses",                                           "count",  1},
    )

  // clang-format on

  // Passed as a function pointer to each RevCore for when they encounter a function that
  // results in a new RevThread being spawned
  std::function<uint32_t()> GetNewTID() {
    return std::function<uint32_t()>( [this]() { return GetNewThreadID(); } );
  }

private:
  unsigned numCores{};     ///< RevCPU: number of RISC-V cores
  unsigned numHarts{};     ///< RevCPU: number of RISC-V cores
  unsigned msgPerCycle{};  ///< RevCPU: number of messages to send per cycle
  //  unsigned          RDMAPerCycle{};  ///< RevCPU: number of RDMA messages per cycle to inject into PAN network
  //  unsigned          testStage{};     ///< RevCPU: controls the PAN Test harness staging
  //  unsigned          testIters{};     ///< RevCPU: the number of message iters for each PAN Test
  RevOpts*              Opts{};     ///< RevCPU: Simulation options object
  RevMem*               Mem{};      ///< RevCPU: RISC-V main memory object
  RevLoader*            Loader{};   ///< RevCPU: RISC-V loader
  std::vector<RevCore*> Procs{};    ///< RevCPU: RISC-V processor objects
  bool*                 Enabled{};  ///< RevCPU: Completion structure

  // Initializes a RevThread object.
  // - Adds it's ThreadID to the ThreadQueue to be scheduled
  void InitThread( std::unique_ptr<RevThread>&& ThreadToInit );

  // Initializes the main thread
  void InitMainThread( uint32_t MainThreadID, uint64_t StartAddr );

  // Adds Thread with ThreadID to AssignedThreads vector for ProcID
  // - Handles updating LSQueue & MarkLoadComplete function pointers
  void AssignThread( std::unique_ptr<RevThread>&& ThreadToAssign, unsigned ProcID );

  // Checks the status of ALL threads that are currently blocked.
  void CheckBlockedThreads();

  // Checks core w/ ProcID to see if it has any available harts to assign work to
  // if it does and there is work to assign (ie. ThreadQueue is not empty)
  // assign it and enable the processor if not already enabled.
  void UpdateThreadAssignments( uint32_t ProcID );

  // Checks for state changes in the threads of a given processor index 'ProcID'
  // and handle appropriately
  void HandleThreadStateChangesForProc( uint32_t ProcID );

  // Checks if a thread with a given Thread ID can proceed (used for pthread_join).
  // it does this by seeing if a given thread's WaitingOnTID has completed
  bool ThreadCanProceed( const std::unique_ptr<RevThread>& Thread );

  // vector of Threads which are ready to be scheduled
  std::vector<std::unique_ptr<RevThread>> ReadyThreads{};

  // List of Threads that are currently blocked (waiting for their WaitingOnTID to be a key in CompletedThreads).
  std::list<std::unique_ptr<RevThread>> BlockedThreads{};

  // Set of Thread IDs and their corresponding RevThread that have completed their execution on this RevCPU
  std::unordered_map<uint32_t, std::unique_ptr<RevThread>> CompletedThreads{};

  // Generates a new Thread ID using the RNG.
  uint32_t GetNewThreadID() { return RevRand( 0, UINT32_MAX ); }

  uint8_t PrivTag{};  ///< RevCPU: private tag locator
  //  uint32_t LToken{};   ///< RevCPU: token identifier for PAN Test

  int address{};  ///< RevCPU: local network address

  unsigned fault_width{};  ///< RevCPU: the width (in bits) for target faults
  // int64_t  fault_range{};  ///< RevCPU: the range of cycles to inject the fault
  int64_t FaultCntr{};  ///< RevCPU: the fault counter

  // uint64_t PrevAddr{};  ///< RevCPU: previous address for handling PAN messages

  bool EnableNIC{};     ///< RevCPU: Flag for enabling the NIC
  bool EnableMemH{};    ///< RevCPU: Enable memHierarchy
  bool EnableCoProc{};  ///< RevCPU: Enable a co-processor attached to all cores

  bool EnableFaults{};       ///< RevCPU: Enable fault injection logic
  bool EnableCrackFaults{};  ///< RevCPU: Enable Crack+Decode Faults
  bool EnableMemFaults{};    ///< RevCPU: Enable memory faults (bit flips)
  bool EnableRegFaults{};    ///< RevCPU: Enable register faults
  bool EnableALUFaults{};    ///< RevCPU: Enable ALU faults

  bool DisableCoprocClock{};  ///< RevCPU: Disables manual coproc clocking

  TimeConverter* timeConverter{};  ///< RevCPU: SST time conversion handler
  SST::Output    output{};         ///< RevCPU: SST output handler

  nicAPI*     Nic{};   ///< RevCPU: Network interface controller
  RevMemCtrl* Ctrl{};  ///< RevCPU: Rev memory controller

  std::vector<RevCoProc*> CoProcs{};  ///< RevCPU: CoProcessor attached to Rev

  SST::Clock::Handler<RevCPU>* ClockHandler{};  ///< RevCPU: Clock Handler

  std::queue<std::pair<uint32_t, char*>> ZeroRqst{};   ///< RevCPU: tracks incoming zero address put requests; pair<Size, Data>
  std::list<std::pair<uint8_t, int>>     TrackTags{};  ///< RevCPU: tracks the outgoing messages; pair<Tag, Dest>
  std::vector<std::tuple<uint8_t, uint64_t, uint32_t>>
    TrackGets{};  ///< RevCPU: tracks the outstanding get messages; tuple<Tag, Addr, Sz>
  std::vector<std::tuple<uint8_t, uint32_t, unsigned, int, uint64_t>> ReadQueue{};  ///< RevCPU: outgoing memory read queue
  ///<         - Tag
  ///<         - Size
  ///<         - Cost
  ///<         - Src
  ///<         - Addr

  //-------------------------------------------------------
  // -- STATISTICS
  //-------------------------------------------------------
  Statistic<uint64_t>* SyncGetSend{};
  Statistic<uint64_t>* SyncPutSend{};
  Statistic<uint64_t>* AsyncGetSend{};
  Statistic<uint64_t>* AsyncPutSend{};
  Statistic<uint64_t>* SyncStreamGetSend{};
  Statistic<uint64_t>* SyncStreamPutSend{};
  Statistic<uint64_t>* AsyncStreamGetSend{};
  Statistic<uint64_t>* AsyncStreamPutSend{};
  Statistic<uint64_t>* ExecSend{};
  Statistic<uint64_t>* StatusSend{};
  Statistic<uint64_t>* CancelSend{};
  Statistic<uint64_t>* ReserveSend{};
  Statistic<uint64_t>* RevokeSend{};
  Statistic<uint64_t>* HaltSend{};
  Statistic<uint64_t>* ResumeSend{};
  Statistic<uint64_t>* ReadRegSend{};
  Statistic<uint64_t>* WriteRegSend{};
  Statistic<uint64_t>* SingleStepSend{};
  Statistic<uint64_t>* SetFutureSend{};
  Statistic<uint64_t>* RevokeFutureSend{};
  Statistic<uint64_t>* StatusFutureSend{};
  Statistic<uint64_t>* SuccessSend{};
  Statistic<uint64_t>* FailedSend{};
  Statistic<uint64_t>* BOTWSend{};
  Statistic<uint64_t>* SyncGetRecv{};
  Statistic<uint64_t>* SyncPutRecv{};
  Statistic<uint64_t>* AsyncGetRecv{};
  Statistic<uint64_t>* AsyncPutRecv{};
  Statistic<uint64_t>* SyncStreamGetRecv{};
  Statistic<uint64_t>* SyncStreamPutRecv{};
  Statistic<uint64_t>* AsyncStreamGetRecv{};
  Statistic<uint64_t>* AsyncStreamPutRecv{};
  Statistic<uint64_t>* ExecRecv{};
  Statistic<uint64_t>* StatusRecv{};
  Statistic<uint64_t>* CancelRecv{};
  Statistic<uint64_t>* ReserveRecv{};
  Statistic<uint64_t>* RevokeRecv{};
  Statistic<uint64_t>* HaltRecv{};
  Statistic<uint64_t>* ResumeRecv{};
  Statistic<uint64_t>* ReadRegRecv{};
  Statistic<uint64_t>* WriteRegRecv{};
  Statistic<uint64_t>* SingleStepRecv{};
  Statistic<uint64_t>* SetFutureRecv{};
  Statistic<uint64_t>* RevokeFutureRecv{};
  Statistic<uint64_t>* StatusFutureRecv{};
  Statistic<uint64_t>* SuccessRecv{};
  Statistic<uint64_t>* FailedRecv{};
  Statistic<uint64_t>* BOTWRecv{};
  // ----- Per Core Statistics
  std::vector<Statistic<uint64_t>*> TotalCycles{};
  std::vector<Statistic<uint64_t>*> CyclesWithIssue{};
  std::vector<Statistic<uint64_t>*> FloatsRead{};
  std::vector<Statistic<uint64_t>*> FloatsWritten{};
  std::vector<Statistic<uint64_t>*> DoublesRead{};
  std::vector<Statistic<uint64_t>*> DoublesWritten{};
  std::vector<Statistic<uint64_t>*> BytesRead{};
  std::vector<Statistic<uint64_t>*> BytesWritten{};
  std::vector<Statistic<uint64_t>*> FloatsExec{};
  std::vector<Statistic<uint64_t>*> TLBMissesPerCore{};
  std::vector<Statistic<uint64_t>*> TLBHitsPerCore{};

  //-------------------------------------------------------
  // -- FUNCTIONS
  //-------------------------------------------------------

  /// RevCPU: decode the fault codes
  void DecodeFaultCodes( const std::vector<std::string>& faults );

  /// RevCPU:: decode the fault width
  void DecodeFaultWidth( const std::string& width );

  /// RevCPU: RevNIC message handler
  void handleMessage( SST::Event* ev );

  /// RevCPU: Creates a unique tag for this message
  uint8_t createTag();

  /// RevCPU: Registers all the internal statistics
  void registerStatistics();

  /// RevCPU: handle fault injection
  void HandleFaultInjection( SST::Cycle_t currentCycle );

  /// RevCPU: handle crack+decode fault injection
  void HandleCrackFault( SST::Cycle_t currentCycle );

  /// RevCPU: handle memory fault
  void HandleMemFault( SST::Cycle_t currentCycle );

  /// RevCPU: handle register fault
  void HandleRegFault( SST::Cycle_t currentCycle );

  /// RevCPU: handle ALU fault
  void HandleALUFault( SST::Cycle_t currentCycle );

  /// RevCPU: updates sst statistics on a per core basis
  void UpdateCoreStatistics( unsigned coreNum );

};  // class RevCPU

}  // namespace SST::RevCPU

#endif

// EOF
