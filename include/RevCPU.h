//
// _RevCPU_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_H_
#define _SST_REVCPU_H_

// -- Standard Headers
#include <vector>
#include <queue>
#include <tuple>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <utility>
#include <string>
#include <vector>
#include <functional>

// -- SST Headers
#include "SST.h"

// -- Rev Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevMemCtrl.h"
#include "RevLoader.h"
#include "RevProc.h"
#include "RevThread.h"
#include "RevNIC.h"
#include "PanNet.h"
#include "PanExec.h"
#include "RevCoProc.h"
#include "RevRand.h"

// -- PAN Common Headers
#include "../common/include/PanAddr.h"

#define _MAX_PAN_TEST_ 11
#define LARGE_PRIME 2147483647

namespace SST::RevCPU{

class RevCPU : public SST::Component{

public:
  /// RevCPU: top-level SST component constructor
  RevCPU( SST::ComponentId_t id, const SST::Params& params );

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

  /// RevCPU: test harness clock tick function
  bool clockTickPANTest( SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // RevCPU Component Registration Data
  // -------------------------------------------------------
  /// RevCPU: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT(
    RevCPU,                             // component class
    "revcpu",                           // component library
    "RevCPU",                           // component name
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "RISC-V SST CPU",
    COMPONENT_CATEGORY_PROCESSOR
    )

  // -------------------------------------------------------
  // RevCPU Component Parameter Data
  // -------------------------------------------------------
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
    {"co_proc", "Co-processor attached to RevProc", "SST::RevCPU::RevSimpleCoProc"},
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
    {"TLBHits",             "TLB hits",                                             "count",  1},
    {"TLBMisses",           "TLB misses",                                           "count",  1},
    {"TLBHitsPerCore",      "TLB hits per core",                                    "count",  1},
    {"TLBMissesPerCore",    "TLB misses per core",                                  "count",  1},
    )

  // Passed as a function pointer to each RevProc for when they encounter a function that
  // results in a new RevThread being spawned
  std::function<uint32_t()> GetNewTID() {
    return std::function<uint32_t()>([this]() { return GetNewThreadID(); });
  }

private:
  unsigned numCores;                  ///< RevCPU: number of RISC-V cores
  unsigned numHarts;                  ///< RevCPU: number of RISC-V cores
  unsigned msgPerCycle;               ///< RevCPU: number of messages to send per cycle
  unsigned RDMAPerCycle;              ///< RevCPU: number of RDMA messages per cycle to inject into PAN network
  unsigned testStage;                 ///< RevCPU: controls the PAN Test harness staging
  unsigned testIters;                 ///< RevCPU: the number of message iters for each PAN Test
  std::string Exe;                    ///< RevCPU: binary executable
  std::string Args;                   ///< RevCPU: argument list
  RevOpts *Opts;                      ///< RevCPU: Simulation options object
  RevMem *Mem;                        ///< RevCPU: RISC-V main memory object
  RevLoader *Loader;                  ///< RevCPU: RISC-V loader
  std::vector<RevProc *> Procs;       ///< RevCPU: RISC-V processor objects
  bool *Enabled;                      ///< RevCPU: Completion structure

  // Global table of threads
  // - The actual RevThread objects live in this table
  // - Only place where the presence of a RevThread on this CPU is guanranteed throughout execution
  std::map<uint32_t, std::shared_ptr<RevThread>> Threads;

  // Oversubscription is not supported in hardware
  // (ie. AssignedThreads.at(i).size() will never exceed the numHarts)
  std::vector<std::unordered_map<uint32_t, std::shared_ptr<RevThread>>> AssignedThreads;

  // Initializes a RevThread object.
  // - Moves it to the 'Threads' map
  // - Adds it's ThreadID to the ThreadQueue to be scheduled
  void InitThread(std::shared_ptr<RevThread>& ThreadToInit);

  // Adds Thread with ThreadID to AssignedThreads vector for ProcID
  // - Handles updating LSQueue & MarkLoadComplete function pointers
  void AssignThread(uint32_t ThreadID, uint32_t ProcID);

  // Sets up arguments for a thread with a given ID and feature set.
  void SetupArgs(uint32_t ThreadIDToSetup, RevFeature* feature);

  // Checks the status of ALL threads that are currently blocked.
  void CheckBlockedThreads();

  // Checks core w/ ProcID to see if it has any available harts to assign work to
  // if it does and there is work to assign (ie. ThreadQueue is not empty)
  // assign it and enable the processor if not already enabled.
  void UpdateThreadAssignments(uint32_t ProcID);

  // Checks for state changes in the threads of a given processor index 'ProcID'
  // and handle appropriately
  void CheckForThreadStateChanges(uint32_t ProcID);

  // Checks for new threads that may have been added to a given processor's NewThreadInfo
  void CheckForNewThreads(uint32_t ProcID);

  // Checks if a thread with a given Thread ID can proceed (used for pthread_join).
  // it does this by seeing if a given thread's WaitingOnTID has completed
  bool ThreadCanProceed(uint32_t TID);

  // Queue of Thread IDs waiting to be assigned. The actual RevThread objects reside in the 'Threads' map.
  std::vector<uint32_t> ThreadQueue = {};

  // Set of Thread IDs that are currently blocked (waiting for their WaitingOnTID to be present in CompletedThreads).
  std::set<uint32_t> BlockedThreads = {};

  // Set of Thread IDs that have completed their execution.
  std::set<uint32_t> CompletedThreads = {};

  // Generates a new Thread ID using the RNG.
  uint32_t GetNewThreadID() { return RevRand(0, UINT32_MAX); }

  uint8_t PrivTag;                    ///< RevCPU: private tag locator
  uint32_t LToken;                    ///< RevCPU: token identifier for PAN Test

  int address;                        ///< RevCPU: local network address

  unsigned fault_width;               ///< RevCPU: the width (in bits) for target faults
  int64_t fault_range;                ///< RevCPU: the range of cycles to inject the fault
  int64_t FaultCntr;                  ///< RevCPU: the fault counter

  uint64_t PrevAddr;                  ///< RevCPU: previous address for handling PAN messages

  bool EnableNIC;                     ///< RevCPU: Flag for enabling the NIC
  bool EnablePAN;                     ///< RevCPU: Flag for enabling the PAN operations
  bool EnablePANTest;                 ///< RevCPU: Flag for enabling the PAN test operations
  bool EnablePANStats;                ///< RevCPU: Flag for enabling PAN statistics
  bool EnableRDMAMBox;                ///< RevCPU: Enable the RDMA Mailbox

  bool EnableMemH;                    ///< RevCPU: Enable memHierarchy
  bool EnableCoProc;                  ///< RevCPU: Enable a co-processor attached to all cores

  bool EnableFaults;                  ///< RevCPU: Enable fault injection logic
  bool EnableCrackFaults;             ///< RevCPU: Enable Crack+Decode Faults
  bool EnableMemFaults;               ///< RevCPU: Enable memory faults (bit flips)
  bool EnableRegFaults;               ///< RevCPU: Enable register faults
  bool EnableALUFaults;               ///< RevCPU: Enable ALU faults

  bool ReadyForRevoke;                ///< RevCPU: Is the CPU ready for revocation?
  bool RevokeHasArrived;              ///< RevCPU: Determines whether the REVOKE command has arrived

  TimeConverter* timeConverter;       ///< RevCPU: SST time conversion handler
  SST::Output output;                 ///< RevCPU: SST output handler

  nicAPI *Nic;                        ///< RevCPU: Network interface controller
  panNicAPI *PNic;                    ///< RevCPU: PAN network interface controller
  PanExec *PExec;                     ///< RevCPU: PAN execution context
  RevMemCtrl *Ctrl;                   ///< RevCPU: Rev memory controller

  std::vector<RevCoProc*> CoProcs;    ///< RevCPU: CoProcessor attached to Rev

  SST::Clock::Handler<RevCPU>* ClockHandler;  ///< RevCPU: Clock Handler

  std::queue<std::pair<panNicEvent *, int>> SendMB;  ///< RevCPU: outgoing command mailbox; pair<Cmd, Dest>
  std::queue<std::pair<uint32_t, char *>> ZeroRqst;  ///< RevCPU: tracks incoming zero address put requests; pair<Size, Data>
  std::list<std::pair<uint8_t, int>> TrackTags;      ///< RevCPU: tracks the outgoing messages; pair<Tag, Dest>
  std::vector<std::tuple<uint8_t,
                         uint64_t,
                         uint32_t>> TrackGets;      ///< RevCPU: tracks the outstanding get messages; tuple<Tag, Addr, Sz>
  std::vector<std::tuple<uint8_t,
                         uint32_t,
                         unsigned,
                         int,
                         uint64_t>> ReadQueue;      ///< RevCPU: outgoing memory read queue
  ///<         - Tag
  ///<         - Size
  ///<         - Cost
  ///<         - Src
  ///<         - Addr

  //-------------------------------------------------------
  // -- STATISTICS
  //-------------------------------------------------------
  Statistic<uint64_t>* SyncGetSend;
  Statistic<uint64_t>* SyncPutSend;
  Statistic<uint64_t>* AsyncGetSend;
  Statistic<uint64_t>* AsyncPutSend;
  Statistic<uint64_t>* SyncStreamGetSend;
  Statistic<uint64_t>* SyncStreamPutSend;
  Statistic<uint64_t>* AsyncStreamGetSend;
  Statistic<uint64_t>* AsyncStreamPutSend;
  Statistic<uint64_t>* ExecSend;
  Statistic<uint64_t>* StatusSend;
  Statistic<uint64_t>* CancelSend;
  Statistic<uint64_t>* ReserveSend;
  Statistic<uint64_t>* RevokeSend;
  Statistic<uint64_t>* HaltSend;
  Statistic<uint64_t>* ResumeSend;
  Statistic<uint64_t>* ReadRegSend;
  Statistic<uint64_t>* WriteRegSend;
  Statistic<uint64_t>* SingleStepSend;
  Statistic<uint64_t>* SetFutureSend;
  Statistic<uint64_t>* RevokeFutureSend;
  Statistic<uint64_t>* StatusFutureSend;
  Statistic<uint64_t>* SuccessSend;
  Statistic<uint64_t>* FailedSend;
  Statistic<uint64_t>* BOTWSend;
  Statistic<uint64_t>* SyncGetRecv;
  Statistic<uint64_t>* SyncPutRecv;
  Statistic<uint64_t>* AsyncGetRecv;
  Statistic<uint64_t>* AsyncPutRecv;
  Statistic<uint64_t>* SyncStreamGetRecv;
  Statistic<uint64_t>* SyncStreamPutRecv;
  Statistic<uint64_t>* AsyncStreamGetRecv;
  Statistic<uint64_t>* AsyncStreamPutRecv;
  Statistic<uint64_t>* ExecRecv;
  Statistic<uint64_t>* StatusRecv;
  Statistic<uint64_t>* CancelRecv;
  Statistic<uint64_t>* ReserveRecv;
  Statistic<uint64_t>* RevokeRecv;
  Statistic<uint64_t>* HaltRecv;
  Statistic<uint64_t>* ResumeRecv;
  Statistic<uint64_t>* ReadRegRecv;
  Statistic<uint64_t>* WriteRegRecv;
  Statistic<uint64_t>* SingleStepRecv;
  Statistic<uint64_t>* SetFutureRecv;
  Statistic<uint64_t>* RevokeFutureRecv;
  Statistic<uint64_t>* StatusFutureRecv;
  Statistic<uint64_t>* SuccessRecv;
  Statistic<uint64_t>* FailedRecv;
  Statistic<uint64_t>* BOTWRecv;
  // ----- Per Core Statistics
  std::vector<Statistic<uint64_t>*> TotalCycles;
  std::vector<Statistic<uint64_t>*> CyclesWithIssue;
  std::vector<Statistic<uint64_t>*> FloatsRead;
  std::vector<Statistic<uint64_t>*> FloatsWritten;
  std::vector<Statistic<uint64_t>*> DoublesRead;
  std::vector<Statistic<uint64_t>*> DoublesWritten;
  std::vector<Statistic<uint64_t>*> BytesRead;
  std::vector<Statistic<uint64_t>*> BytesWritten;
  std::vector<Statistic<uint64_t>*> FloatsExec;
  std::vector<Statistic<uint64_t>*> TLBMissesPerCore;
  std::vector<Statistic<uint64_t>*> TLBHitsPerCore;

  //-------------------------------------------------------
  // -- FUNCTIONS
  //-------------------------------------------------------

  /// RevCPU: initializes the PAN NIC tables
  void initNICMem();

  /// RevCPU: decode the fault codes
  void DecodeFaultCodes(const std::vector<std::string>& faults);

  /// RevCPU:: decode the fault width
  void DecodeFaultWidth(const std::string& width);

  /// RevCPU: executes the PAN test harness
  void ExecPANTest();

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

  /// RevCPU: handles the PAN RDMA mailbox
  bool PANProcessRDMAMailbox();

  /// RevCPU: handles the PAN zero address put requests
  bool processPANZeroAddr();

  /// RevCPU: converts an RDMA payload to a panNicEvent command
  bool PANConvertRDMAtoEvent(uint64_t Addr, panNicEvent *event);

  /// RevCPU: Creates a unique tag for this message
  uint8_t createTag();

  /// RevCPU: Registers all the internal statistics
  void registerStatistics();

  // RevCPU: Register send message statistics
  void registerSendCmd(panNicEvent *event);

  /// RevCPU: handle a zero address Put command where the NIC chooses the destination buffer
  bool PANHandleZeroAddrPut(uint32_t Size, void *Data);

  /// RevCPU: handle fault injection
  void HandleFaultInjection(SST::Cycle_t currentCycle);

  /// RevCPU: handle crack+decode fault injection
  void HandleCrackFault(SST::Cycle_t currentCycle);

  /// RevCPU: handle memory fault
  void HandleMemFault(SST::Cycle_t currentCycle);

  /// RevCPU: handle register fault
  void HandleRegFault(SST::Cycle_t currentCycle);

  /// RevCPU: handle ALU fault
  void HandleALUFault(SST::Cycle_t currentCycle);

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

  /// RevCPU: Signal the host thread of a completed round trip message
  void PANSignalMsgRecv(uint8_t tag, uint64_t sig);

  /// RevCPU: Handle successful response
  void PANHandleSuccess(panNicEvent *event);

  /// RevCPU: Handle failed response
  void PANHandleFailed(panNicEvent *event);

  /// RevCPU: construct a failed PAN response command due to a token failure
  ///         Implicitly pushes the failure command onto the send mailbox
  void PANBuildFailedToken(panNicEvent *event);

  /// RevCPU: construct a generic successful PAN response command.
  ///         Implicitly pushes the success command onto the send mailbox
  void PANBuildRawSuccess(panNicEvent *event);

  /// RevCPU: construct a generic success command response in the target rtn packet.
  ///         Does not push onto the send mailbox
  void PANBuildBasicSuccess(panNicEvent *event, panNicEvent *rtn);

  /// RevCPU: updates sst statistics on a per core basis
  void UpdateCoreStatistics(unsigned coreNum);

}; // class RevCPU

} // namespace SST::RevCPU

#endif

// EOF
