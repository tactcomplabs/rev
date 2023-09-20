//
// _RevCPU_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevCPU.h"
#include <cmath>
#include "RevMem.h"
#include "RevThread.h"

using namespace SST::RevCPU;

const char splash_msg[] = "\
\n\
*******                   \n\
/**////**                  \n\
/**   /**   *****  **    **\n\
/*******   **///**/**   /**\n\
/**///**  /*******//** /** \n\
/**  //** /**////  //****  \n\
/**   //**//******  //**   \n\
//     //  //////    //    \n\
\n\
";

const char pan_splash_msg[] = "\
\n\
       __|__\n\
--@--@--(_)--@--@--\n\
\n\
    PAN PAN PAN!\n\
";

RevCPU::RevCPU( SST::ComponentId_t id, const SST::Params& params )
  : SST::Component(id), testStage(0), PrivTag(0), address(-1), PrevAddr(_PAN_RDMA_MAILBOX_),
    EnableNIC(false), EnablePAN(false), EnablePANStats(false), EnableMemH(false),
    ReadyForRevoke(false), Nic(nullptr), PNic(nullptr), PExec(nullptr), Ctrl(nullptr),
    ClockHandler(nullptr) {

  const int Verbosity = params.find<int>("verbose", 0);

  // Initialize the output handler
  output.init("RevCPU[" + getName() + ":@p:@t]: ", Verbosity, 0, SST::Output::STDOUT);

  // Determine whether we're running the test harness
  EnablePANTest = params.find<bool>("enable_test", 0);

  // Register a new clock handler
  {
    const std::string cpuClock = params.find<std::string>("clock", "1GHz");
    if( EnablePANTest ){
      ClockHandler = new SST::Clock::Handler<RevCPU>(this, &RevCPU::clockTickPANTest);
      timeConverter = registerClock(cpuClock, ClockHandler);
      testIters = params.find<unsigned>("testIters", 255);
    }else{
      ClockHandler = new SST::Clock::Handler<RevCPU>(this, &RevCPU::clockTick);
      timeConverter = registerClock(cpuClock, ClockHandler);
    }
  }

  // Inform SST to wait until we authorize it to exit
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Derive the simulation parameters
  // We must always derive the number of cores before initializing the options
  // If the PAN tests are enabled, override the number cores and force them to '0'
  numCores = params.find<unsigned>("numCores", "1");
  if( EnablePANTest )
    numCores = 1; // force the PAN test to use a single core
  output.verbose(CALL_INFO, 1, 0, "Building Rev with %u cores\n", numCores);

  // read the binary executable name
  Exe = params.find<std::string>("program", "a.out");

  // read the program arguments
  Args = params.find<std::string>("args", "");

  // Create the options object
  // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
  Opts = new RevOpts(numCores,Verbosity);
  if( !Opts )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RevOpts object\n" );

  // Initialize the remaining options
  {
    std::vector<std::string> startAddrs;
    params.find_array<std::string>("startAddr", startAddrs);
    if( !Opts->InitStartAddrs( startAddrs ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the starting addresses\n" );

    std::vector<std::string> startSyms;
    params.find_array<std::string>("startSymbol", startSyms);
    if( !Opts->InitStartSymbols( startSyms ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initalized the starting symbols\n" );

    std::vector<std::string> machModels;
    params.find_array<std::string>("machine", machModels);
    if( !Opts->InitMachineModels( machModels ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the machine models\n" );

    std::vector<std::string> instTables;
    params.find_array<std::string>("table",instTables);
    if( !Opts->InitInstTables( instTables ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the instruction tables\n" );

    std::vector<std::string> memCosts;
    params.find_array<std::string>("memCost",memCosts);
    if( !Opts->InitMemCosts( memCosts ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory latency range\n" );

    std::vector<std::string> prefetchDepths;
    params.find_array<std::string>("prefetchDepth",prefetchDepths);
    if( !Opts->InitPrefetchDepth( prefetchDepths) )
      output.fatal(CALL_INFO, -1, "Error: failed to initalize the prefetch depth\n" );
  }

  // See if we should load the network interface controller
  EnableNIC = params.find<bool>("enable_nic", 0);

  if( EnableNIC ){
    // Look up the network component

    Nic = loadUserSubComponent<nicAPI>("nic");

    // check to see if the nic was loaded.  if not, DO NOT load an anonymous endpoint
    if(!Nic)
      output.fatal(CALL_INFO, -1, "Error: no NIC object loaded into RevCPU\n");

    Nic->setMsgHandler(new Event::Handler<RevCPU>(this, &RevCPU::handleMessage));

    // record the number of injected messages per cycle
    msgPerCycle = params.find<unsigned>("msgPerCycle", 1);
  }

  // See if we should the load PAN network interface controller
  EnablePAN = params.find<bool>("enable_pan", 0);
  EnablePANStats = params.find<bool>("enable_pan_stats", 0);

  if( EnablePAN ){
    // Look up the network component

    PNic = loadUserSubComponent<panNicAPI>("pan_nic");

    // check to see if the nic was loaded.  if not, DO NOT load an anonymous endpoint
    if(!PNic)
      output.fatal(CALL_INFO, -1, "Error: no PAN NIC object loaded into RevCPU\n");

    PNic->setMsgHandler(new Event::Handler<RevCPU>(this, &RevCPU::handlePANMessage));

    // setup the PAN target device execution context
    if( !PNic->IsHost() ){
      // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
      PExec = new PanExec();
      if( PExec == nullptr ){
      for( unsigned i=0; i<Procs.size(); i++ ){
        Procs[i]->SetExecCtx(PExec);
      }
    }
    RevokeHasArrived = false;
    }else{
      RevokeHasArrived = true;
    }

    // record the number of injected messages per cycle
    msgPerCycle = params.find<unsigned>("msgPerCycle", 1);
    RDMAPerCycle = params.find<unsigned>("RDMAPerCycle", 1);
    EnableRDMAMBox = params.find<bool>("enableRDMAMbox",1);

    if( EnablePANStats )
      registerStatistics();
  }else{
    RevokeHasArrived = true;
  }

  // See if we should load the test harness as opposed to a binary payload
  if( EnablePANTest && (!EnablePAN) ){
    output.fatal(CALL_INFO, -1, "Error: enabling PAN tests requires a pan_nic");
  }

  // Look for the fault injection logic
  EnableFaults = params.find<bool>("enable_faults", 0);
  if( EnableFaults ){
    std::vector<std::string> faults;
    params.find_array<std::string>("faults",faults);
    DecodeFaultCodes(faults);

    std::string width = params.find<std::string>("fault_width","1");
    DecodeFaultWidth(width);

    fault_width = params.find<int64_t>("fault_range","65536");
    FaultCntr = fault_width;
  }

  // Create the memory object
  const uint64_t memSize = params.find<unsigned long>("memSize", 1073741824);
  EnableMemH = params.find<bool>("enable_memH", 0);
  if( !EnableMemH ){
    // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
    Mem = new RevMem( memSize, Opts,  &output );
    if( !Mem )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory object\n" );
  }else{
    if( EnablePAN )
      output.fatal(CALL_INFO, -1, "Error: PAN does not currently support memHierarchy\n");

    Ctrl = loadUserSubComponent<RevMemCtrl>("memory");
    if( !Ctrl )
      output.fatal(CALL_INFO, -1, "Error : failed to inintialize the memory controller subcomponent\n");

    // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
    Mem = new RevMem( memSize, Opts, Ctrl, &output );
    if( !Mem )
      output.fatal(CALL_INFO, -1, "Error : failed to initialize the memory object\n" );

    if( EnableFaults )
      output.verbose(CALL_INFO, 1, 0, "Warning: memory faults cannot be enabled with memHierarchy support\n");
  }

  // Set TLB Size
  const uint64_t tlbSize = params.find<unsigned long>("tlbSize", 512);
  Mem->SetTLBSize(tlbSize);

  // Set max heap size
  const uint64_t maxHeapSize = params.find<unsigned long>("maxHeapSize", memSize/4);
  Mem->SetMaxHeapSize(maxHeapSize);

  // Load the binary into memory
  // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
  Loader = new RevLoader( Exe, Args, Mem, &output );
  if( !Loader ){
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RISC-V loader\n" );
  }

  // std::cout << "Loader: " << Loader << std::endl;

  Opts->SetArgs(Loader->GetArgv());

  AssignedThreads.resize(numCores);
  EnableCoProc = params.find<bool>("enableCoProc", 0);
  if(EnableCoProc){
    // Create the co-processor objects
    for( unsigned i=0; i<numCores; i++){
      RevCoProc* CoProc = loadUserSubComponent<RevCoProc>("co_proc");
        if (!CoProc) {
          output.fatal(CALL_INFO, -1, "Error : failed to inintialize the co-processor subcomponent\n");
        }
        CoProcs.push_back(CoProc);
    }
    // Create the processor objects
    Procs.reserve(Procs.size() + numCores);
    for( unsigned i=0; i<numCores; i++ ){
      Procs.push_back( new RevProc( i, Opts, Mem, Loader, AssignedThreads.at(i), this->GetNewTID(), CoProcs[i], &output ) );
    }
  }else{
    // Create the processor objects
    Procs.reserve(Procs.size() + numCores);
    for( unsigned i=0; i<numCores; i++ ){
      Procs.push_back( new RevProc( i, Opts, Mem, Loader, AssignedThreads.at(i), this->GetNewTID(), NULL, &output ) );
    }
  }

  // setup the per-proc statistics
  TotalCycles.reserve(TotalCycles.size() + numCores);
  CyclesWithIssue.reserve(CyclesWithIssue.size() + numCores);
  FloatsRead.reserve(FloatsRead.size() + numCores);
  FloatsWritten.reserve(FloatsWritten.size() + numCores);
  DoublesRead.reserve(DoublesRead.size() + numCores);
  DoublesWritten.reserve(DoublesWritten.size() + numCores);
  BytesRead.reserve(BytesRead.size() + numCores);
  BytesWritten.reserve(BytesWritten.size() + numCores);
  FloatsExec.reserve(FloatsExec.size() + numCores);
  TLBHitsPerCore.reserve(TLBHitsPerCore.size() + numCores);
  TLBMissesPerCore.reserve(TLBMissesPerCore.size() + numCores);

  for(unsigned s = 0; s < numCores; s++){
    TotalCycles.push_back(registerStatistic<uint64_t>("TotalCycles", "core_" + std::to_string(s)));
    CyclesWithIssue.push_back(registerStatistic<uint64_t>("CyclesWithIssue", "core_" + std::to_string(s)));
    FloatsRead.push_back( registerStatistic<uint64_t>("FloatsRead", "core_" + std::to_string(s)));
    FloatsWritten.push_back( registerStatistic<uint64_t>("FloatsWritten", "core_" + std::to_string(s)));
    DoublesRead.push_back( registerStatistic<uint64_t>("DoublesRead", "core_" + std::to_string(s)));
    DoublesWritten.push_back( registerStatistic<uint64_t>("DoublesWritten", "core_" + std::to_string(s)));
    BytesRead.push_back( registerStatistic<uint64_t>("BytesRead", "core_" + std::to_string(s)));
    BytesWritten.push_back( registerStatistic<uint64_t>("BytesWritten", "core_" + std::to_string(s)));
    FloatsExec.push_back( registerStatistic<uint64_t>("FloatsExec", "core_" + std::to_string(s)));
    TLBHitsPerCore.push_back( registerStatistic<uint64_t>("TLBHitsPerCore", "core_" + std::to_string(s)));
    TLBMissesPerCore.push_back( registerStatistic<uint64_t>("TLBMissesPerCore", "core_" + std::to_string(s)));
  }


  uint32_t MainThreadID = 1;
  // uint32_t MainParentThreadID = 0;  // commented out to prevent compiler warning

  // set the pc
  uint64_t StartAddr = 0x00ull;
  if( !Opts->GetStartAddr( id, StartAddr ) )
    output.fatal(CALL_INFO, -1, "Error: failed to init the start address for the main thread\n");
  std::string StartSymbol = "main";
  //std::string StartSymbol = "_start";
  if( StartAddr == 0x00ull ){
    if( !Opts->GetStartSymbol( id, StartSymbol ) )
      output.fatal(CALL_INFO, -1,
                    "Error: failed to init the start symbol address for main thread=\n");

    StartAddr = Loader->GetSymbolAddr(StartSymbol);
  }
  if( StartAddr == 0x00ull ){
    // load "main" symbol
    StartAddr = Loader->GetSymbolAddr("main");
    //StartAddr = loader->GetSymbolAddr("_start");
    if( StartAddr == 0x00ull ){
      output.fatal(CALL_INFO, -1,
                    "Error: failed to auto discover address for <main> for main thread\n");
    }
  }

  // Create the first copy of the TLS Segment
  Threads.emplace(MainThreadID,
                  std::make_shared<RevThread>(0,
                                              Mem->GetStackTop(),
                                              StartAddr,
                                              Mem->GetThreadMemSegs().front()));

  Threads.at(MainThreadID)->SetThreadID(MainThreadID);

  std::cout << "First Thread Info: " << std::endl;
  std::cout << *Threads[MainThreadID] << std::endl;

  Threads.at(MainThreadID)->GetRegFile()->RV32[3] = (uint32_t)(Loader->GetSymbolAddr("__global_pointer$"));
  Threads.at(MainThreadID)->GetRegFile()->RV64[3] = Loader->GetSymbolAddr("__global_pointer$");

  Threads.at(MainThreadID)->GetRegFile()->RV32[8] = Threads.at(MainThreadID)->GetRegFile()->RV32[3];
  Threads.at(MainThreadID)->GetRegFile()->RV64[8] = Threads.at(MainThreadID)->GetRegFile()->RV64[3];

  Threads.at(MainThreadID)->GetRegFile()->cost = 0;

  // Assign the first thread
  // AssignedThreads.front().emplace_back(Threads[1]);
  ThreadQueue.emplace_back(MainThreadID);

  // setup the PAN execution contexts
  if( EnablePAN ){
    // setup the PAN target device execution context
    if( !PNic->IsHost() ){
      PExec = new PanExec();
      for( unsigned i=0; i<Procs.size(); i++ ){
        Procs[i]->SetExecCtx(PExec);
      }
    }
  }

  // Create the completion array
  Enabled = new bool [numCores];
  // Starting at 1 because the first Proc executes the main thread
  for( unsigned i=0; i<numCores; i++ ){
    // FIXME: This may break PAN
    Enabled[i] = false;
  }

  {
    const unsigned Splash = params.find<bool>("splash",0);

    if( Splash > 0 ){
      if( EnablePANTest )
        output.verbose(CALL_INFO,1,0,pan_splash_msg);
      else
        output.verbose(CALL_INFO,1,0,splash_msg);
    }
  }

  // Done with initialization
  if( EnablePANTest )
    output.verbose(CALL_INFO, 1, 0, "Initialization of PANTest harness complete.\n");
  else
    output.verbose(CALL_INFO, 1, 0, "Initialization of RevCPUs complete.\n");
}

RevCPU::~RevCPU(){

  // delete the competion array
  delete[] Enabled;

  // delete the processors objects
  for( size_t i = 0; i < Procs.size(); i++ ){
    delete Procs[i];
  }

  for (size_t i = 0; i < CoProcs.size(); i++){
    delete CoProcs[i];
  }

    delete PExec;

  // delete the memory controller if present
    delete Ctrl;

  // delete the memory object
  delete Mem;

  // delete the loader object
    delete Loader;

  // delete the options object
  delete Opts;

  // delete the clock handler object
  delete ClockHandler;
}

void RevCPU::DecodeFaultWidth(const std::string& width){
  fault_width = 1;  // default to single bit failures

  if( width == "single" ){
    fault_width = 1;
  }else if( width == "word" ){
    fault_width = 8;
  }else{
    fault_width = std::stoi(width);
  }

  if( fault_width > 64 ){
    output.fatal(CALL_INFO, -1, "Fault width must be <= 64 bits");
  }
}

void RevCPU::DecodeFaultCodes(const std::vector<std::string>& faults){
  if( faults.empty() ){
    output.fatal(CALL_INFO, -1, "No fault codes defined");
  }

  EnableCrackFaults = EnableMemFaults = EnableRegFaults = EnableALUFaults = false;

  for(auto& fault : faults){
    if( fault == "decode"){
      EnableCrackFaults = true;
    }else if( fault == "mem"){
      EnableMemFaults = true;
    }else if( fault == "reg"){
      EnableRegFaults = true;
    }else if( fault == "alu"){
      EnableALUFaults = true;
    }else if( fault == "all" ){
      EnableCrackFaults = EnableMemFaults =  EnableRegFaults = EnableALUFaults = true;
    }else{
      output.fatal( CALL_INFO, -1, "Undefined fault code: %s", fault.c_str() );
    }
  }
}

void RevCPU::initNICMem(){
  output.verbose(CALL_INFO,1,0,"Initializing NIC memory.\n");
  #if 1
  // See PanAddr.h
  Mem->AddMemSegAt(_PAN_COMPLETION_ADDR_, sizeof(uint64_t));
  Mem->AddMemSegAt(_PAN_COMPLETION_ADDR_, sizeof(MBoxEntry) * _PAN_RDMA_MAX_ENTRIES_);
  Mem->AddMemSegAt(_PAN_PE_TABLE_ADDR_,   sizeof(PEMap) * _PAN_PE_TABLE_MAX_ENTRIES_);
  Mem->AddMemSegAt(_PAN_XFER_BUF_ADDR_,   sizeof(PRTIME_XFER) * _PAN_XFER_BUF_);
  #endif
  // init all the entries to -1
  uint64_t ptr = _PAN_PE_TABLE_ADDR_;
  uint64_t host = 2;
  int64_t id = -1;
  for( unsigned i=0; i<_PAN_PE_TABLE_MAX_ENTRIES_; i++ ){
    Mem->Write(0, ptr, id);
    Mem->Write(0, ptr+8, host);
    ptr += sizeof(PEMap);
  }
  ptr = _PAN_PE_TABLE_ADDR_;

  // the first entry in the table is our own, then its
  // all the other nodes sequentially
  id = PNic->getAddress();
  Mem->Write(0, ptr, id);
  ptr += 8;
  host = PNic->IsHost();
  Mem->Write(0, ptr, host);
  ptr += 8;

  output.verbose(CALL_INFO, 4, 0, "--> MY_PE = %" PRId64 "; IS_HOST = %" PRIu64 "\n",
                 id, host );

  for( unsigned i=0; i<PNic->getNumPEs(); i++ ){
    id = PNic->getHostFromIdx(i);
    Mem->Write(0, ptr, id);
    ptr += 8;
    host = PNic->IsRemoteHost(id);
    output.verbose(CALL_INFO, 4, 0, "--> REMOTE_PE = %" PRId64 "; IS_HOST = %" PRIu64 "\n",
                  id, host);
    Mem->Write(0, ptr, host);
    ptr += 8;
  }
}

void RevCPU::registerStatistics(){
  SyncGetSend = registerStatistic<uint64_t>("SyncGetSend");
  SyncPutSend = registerStatistic<uint64_t>("SyncPutSend");
  AsyncGetSend = registerStatistic<uint64_t>("AsyncGetSend");
  AsyncPutSend = registerStatistic<uint64_t>("AsyncPutSend");
  SyncStreamGetSend = registerStatistic<uint64_t>("SyncStreamGetSend");
  SyncStreamPutSend = registerStatistic<uint64_t>("SyncStreamPutSend");
  AsyncStreamGetSend = registerStatistic<uint64_t>("AsyncStreamGetSend");
  AsyncStreamPutSend = registerStatistic<uint64_t>("AsyncStreamPutSend");
  ExecSend = registerStatistic<uint64_t>("ExecSend");
  StatusSend = registerStatistic<uint64_t>("StatusSend");
  CancelSend = registerStatistic<uint64_t>("CancelSend");
  ReserveSend = registerStatistic<uint64_t>("ReserveSend");
  RevokeSend = registerStatistic<uint64_t>("RevokeSend");
  HaltSend = registerStatistic<uint64_t>("HaltSend");
  ResumeSend = registerStatistic<uint64_t>("ResumeSend");
  ReadRegSend = registerStatistic<uint64_t>("ReadRegSend");
  WriteRegSend = registerStatistic<uint64_t>("WriteRegSend");
  SingleStepSend = registerStatistic<uint64_t>("SingleStepSend");
  SetFutureSend = registerStatistic<uint64_t>("SetFutureSend");
  RevokeFutureSend = registerStatistic<uint64_t>("RevokeFutureSend");
  StatusFutureSend = registerStatistic<uint64_t>("StatusFutureSend");
  SuccessSend = registerStatistic<uint64_t>("SuccessSend");
  FailedSend = registerStatistic<uint64_t>("FailedSend");
  BOTWSend = registerStatistic<uint64_t>("BOTWSend");
  SyncGetRecv = registerStatistic<uint64_t>("SyncGetRecv");
  SyncPutRecv = registerStatistic<uint64_t>("SyncPutRecv");
  AsyncGetRecv = registerStatistic<uint64_t>("AsyncGetRecv");
  AsyncPutRecv = registerStatistic<uint64_t>("AsyncPutRecv");
  SyncStreamGetRecv = registerStatistic<uint64_t>("SyncStreamGetRecv");
  SyncStreamPutRecv = registerStatistic<uint64_t>("SyncStreamPutRecv");
  AsyncStreamGetRecv = registerStatistic<uint64_t>("AsyncStreamGetRecv");
  AsyncStreamPutRecv = registerStatistic<uint64_t>("AsyncStreamPutRecv");
  ExecRecv = registerStatistic<uint64_t>("ExecRecv");
  StatusRecv = registerStatistic<uint64_t>("StatusRecv");
  CancelRecv = registerStatistic<uint64_t>("CancelRecv");
  ReserveRecv = registerStatistic<uint64_t>("ReserveRecv");
  RevokeRecv = registerStatistic<uint64_t>("RevokeRecv");
  HaltRecv = registerStatistic<uint64_t>("HaltRecv");
  ResumeRecv = registerStatistic<uint64_t>("ResumeRecv");
  ReadRegRecv = registerStatistic<uint64_t>("ReadRegRecv");
  WriteRegRecv = registerStatistic<uint64_t>("WriteRegRecv");
  SingleStepRecv = registerStatistic<uint64_t>("SingleStepRecv");
  SetFutureRecv = registerStatistic<uint64_t>("SetFutureRecv");
  RevokeFutureRecv = registerStatistic<uint64_t>("RevokeFutureRecv");
  StatusFutureRecv = registerStatistic<uint64_t>("StatusFutureRecv");
  SuccessRecv = registerStatistic<uint64_t>("SuccessRecv");
  FailedRecv = registerStatistic<uint64_t>("FailedRecv");
  BOTWRecv = registerStatistic<uint64_t>("BOTWRecv");
}

void RevCPU::setup(){
  if( EnableNIC ){
    Nic->setup();
    address = Nic->getAddress();
  }
  if( EnablePAN ){
    PNic->setup();
    address = PNic->getAddress();
    initNICMem();
  }
  if( EnableMemH ){
    Ctrl->setup();
  }
}

void RevCPU::finish(){
}

void RevCPU::init( unsigned int phase ){
  if( EnableNIC )
    Nic->init(phase);
  if( EnablePAN )
    PNic->init(phase);
  if( EnableMemH )
    Ctrl->init(phase);
}

void RevCPU::handleMessage(Event *ev){
  nicEvent *event = static_cast<nicEvent*>(ev);
  // -- RevNIC: This is where you can unpack and handle the data payload
  delete event;
}

//
// This is the PAN Network Transport Module Handler
//
void RevCPU::handlePANMessage(Event *ev){
  panNicEvent *event = static_cast<panNicEvent*>(ev);

  if( PNic->IsHost() ){
    // if we are a host device, only decode host-type messages
    handleHostPANMessage(event);
  }else{
    // otherwise decode all the messages
    handleNetPANMessage(event);
  }

  delete event;
}

void RevCPU::PANSignalMsgRecv(uint8_t tag, uint64_t sig){
  unsigned iter = 0;
  uint64_t Addr = _PAN_RDMA_MAILBOX_;
  uint64_t Payload[3];
  uint64_t TagPayload  = 0x00ull;
  uint8_t TmpTag = 0x0;
  bool done = false;

  while( !done ){
    //
    // retrive the 'iter' 24-byte mailbox payload
    // each payload is configured as follows:
    //      ADDR + 16          ADDR + 8             ADDR
    // [  EVENT POINTER  ][      DEST       ][      VALID      ]
    //
    //

    // Stage 1: Read the memory
    Mem->ReadMem(Addr,24,&Payload[0]);

    // Stage 2: Check to see if the tag matches
    TagPayload = Mem->ReadU64(Payload[2]);
    TmpTag     = (uint8_t)( (TagPayload >> 32) & 0b11111111 );

    // Stage 3: If the tag matches and the mailbox is in an
    //          "injected" state, write the value
    if( (TmpTag == tag) && (Payload[0] == _PAN_ENTRY_INJECTED_) ){
      Payload[0] = sig;
      Mem->WriteMem(0, Addr,8,&Payload[0]);
      done = true;
    }

    Payload[0] = 0x00ull;
    Payload[1] = 0x00ull;
    Payload[2] = 0x00ull;
    TagPayload = 0x00ull;
    iter++;
    Addr+=24;

    if( iter == _PAN_RDMA_MAX_ENTRIES_ ){
      done = true;
    }
  } // end while
}

void RevCPU::PANHandleSuccess(panNicEvent *event){
  // search for the tag in the tag list
  std::pair Entry{event->getTag(), event->getSrc()};
  auto it = std::find(TrackTags.begin(),TrackTags.end(),Entry);
  if( it == TrackTags.end() ){
    // nothing found, raise an error
    output.fatal(CALL_INFO, -1,
                 "Error: failed to find matching tag and source identifier for incoming message: tag=%d; src=%d\n",
                 event->getTag(),
                 event->getSrc());
    return ;  // should not reach this
  }

  // search for the tag in the outstanding get list
  for( auto GetIter = TrackGets.begin(); GetIter != TrackGets.end(); ++GetIter ){
    if( std::get<0>(*GetIter) == event->getTag() ){
      // found a valid entry; setup the memory write
      uint64_t *Data = new uint64_t [event->getNumBlocks(std::get<2>(*GetIter))];
      Mem->WriteMem(0,
                    std::get<1>(*GetIter),
                    std::get<2>(*GetIter),
                    Data);
      delete[] Data;

      // erase the entry
      TrackGets.erase(GetIter);
      if( TrackGets.size() == 0 )
        break;
    }
  }

  // Signal the host thread of the message completion
  PANSignalMsgRecv(event->getTag(),_PAN_ENTRY_DONE_SUCCESS_);

  output.verbose(CALL_INFO, 8, 0,
                 "SUCCESS RESPONSE: Found matching tag and source identifier for incoming message: tag=%d; src=%d\n",
                 event->getTag(),
                 event->getSrc());

  // remove the entry
  TrackTags.erase(it);
}

void RevCPU::PANHandleFailed(panNicEvent *event){
  // search for the tag in the tag list
  std::pair<uint8_t,int> Entry = std::make_pair(event->getTag(),
                                                event->getSrc());
  auto it = std::find(TrackTags.begin(),TrackTags.end(),Entry);
  if( it == TrackTags.end() ){
    // nothing found, raise an error
    output.fatal(CALL_INFO, -1,
                 "Error: failed to find matching tag and source identifier for incoming message: tag=%d; src=%d\n",
                 event->getTag(),
                 event->getSrc());
    return ;  // should not reach this
  }

  // Signal the host thread of the message completion
  PANSignalMsgRecv(event->getTag(),_PAN_ENTRY_DONE_FAILED_);

  output.verbose(CALL_INFO, 8, 0,
                 "FAILED RESPONSE: Found matching tag and source identifier for incoming message: tag=%d; src=%d\n",
                 event->getTag(),
                 event->getSrc());

  // remove the entry
  TrackTags.erase(it);
}

bool RevCPU::PANHandleZeroAddrPut(uint32_t Size, void *Data){
  output.verbose(CALL_INFO, 5, 0, "Handling Zero Address Put Commands\n");
  char *tmp = (char *)(Data);
  char *NewData = new char [Size];
  for( uint32_t i=0; i<Size; i++ ){
    NewData[i] = tmp[i];
  }
  ZeroRqst.push(std::make_pair(Size,NewData));
  return true;
}

void RevCPU::PANBuildFailedToken(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Building failed return packet\n");
  panNicEvent *FEvent = new panNicEvent(getName());
  FEvent->setSrc(address);
  if( !FEvent->buildFailed(event->getToken(),event->getTag()) ){
    output.fatal(CALL_INFO, -1,
                 "Error: failed to construct token failure command for tag=%d\n",
                 event->getTag());
  }
  SendMB.push(std::make_pair(FEvent,event->getSrc()));
}

void RevCPU::PANBuildRawSuccess(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Building success return packet\n");
  panNicEvent *SEvent = new panNicEvent(getName());
  SEvent->setSrc(address);
  if( !SEvent->buildSuccess(event->getToken(),event->getTag()) ){
    output.fatal(CALL_INFO, -1,
                 "Error: failed to construct token success command for tag=%d\n",
                 event->getTag());
  }
  SendMB.push(std::make_pair(SEvent,event->getSrc()));
}

void RevCPU::PANBuildBasicSuccess(panNicEvent *orig, panNicEvent *rtn){
  if( !rtn ){
    output.fatal(CALL_INFO, -1,
                 "Error : new event command packet is null : %d\n",
                 orig->getTag());
  }
  rtn->setSrc(address);
  if( !rtn->buildSuccess(orig->getToken(),orig->getTag()) ){
    output.fatal(CALL_INFO, -1,
                 "Error: failed to construct token success command for tag=%d\n",
                 orig->getTag());
  }
}

void RevCPU::PANHandleSyncGet(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN SYNCGET Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
    }
  }

  // push an event entry back onto the ReadQueue
  ReadQueue.push_back(std::make_tuple(event->getTag(),
                                      event->getSize(),
                                      Procs[0]->RandCost(),
                                      event->getSrc(),
                                      event->getAddr()));

  // we do not create a message here, it will be created later
}

void RevCPU::PANHandleSyncPut(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN SYNCPUT Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // check for zero address
  if( event->getAddr() == 0x00ull ){
    // handle the special zero put messages
    if( !PANHandleZeroAddrPut(Size, Data) ){
      delete[] Data;
      PANBuildFailedToken(event);
    }
  }else if( !Mem->WriteMem(0, event->getAddr(), Size, Data) ){
    delete[] Data;
    PANBuildFailedToken(event);
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleAsyncGet(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN ASYNCGET Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // push an event entry back onto the ReadQueue
  ReadQueue.push_back(std::make_tuple(event->getTag(),
                                      event->getSize(),
                                      Procs[0]->RandCost(),
                                      event->getSrc(),
                                      event->getAddr()));

  // we do not create a message here, it will be created later
}

void RevCPU::PANHandleAsyncPut(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN ASYNCPUT Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // check for zero address
  if( event->getAddr() == 0x00ull ){
    // handle the special zero put messages
    if( !PANHandleZeroAddrPut(Size, Data) ){
      delete[] Data;
      PANBuildFailedToken(event);
    }
  }else if( !Mem->WriteMem(0, event->getAddr(), Size, Data) ){
    delete[] Data;
    PANBuildFailedToken(event);
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleSyncStreamGet(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN SYNCSTREAMGET Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // push an event entry back onto the ReadQueue
  ReadQueue.push_back(std::make_tuple(event->getTag(),
                                      event->getSize(),
                                      Procs[0]->RandCost(),
                                      event->getSrc(),
                                      event->getAddr()));

  // we do not create a message here, it will be created later
}

void RevCPU::PANHandleSyncStreamPut(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN SYNCSTREAMPUT Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // write it to memory
  if( !Mem->WriteMem(0, event->getAddr(), Size, Data) ){
    delete[] Data;
    PANBuildFailedToken(event);
    return ;
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleAsyncStreamGet(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN ASYNCSTREAMGET Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // push an event entry back onto the ReadQueue
  ReadQueue.push_back(std::make_tuple(event->getTag(),
                                      event->getSize(),
                                      Procs[0]->RandCost(),
                                      event->getSrc(),
                                      event->getAddr()));

  // we do not create a message here, it will be created later
}

void RevCPU::PANHandleAsyncStreamPut(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN ASYNCSTREAMPUT Request\n");
  if( !PNic->IsHost() ){
    if( !PNic->CheckToken(event->getToken()) ){
      // send failed response
      PANBuildFailedToken(event);
      return ;
    }
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // write it to memory
  if( !Mem->WriteMem(0, event->getAddr(), Size, Data) ){
    delete[] Data;
    PANBuildFailedToken(event);
    return ;
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleExec(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN EXEC Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  unsigned Idx = 0;
  if( !PExec->AddEntry(event->getAddr(),&Idx) ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Idx);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleStatus(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN STATUS Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }
  unsigned Idx = event->getSize();
  PanExec::PanStatus Status = PExec->StatusEntry(Idx);
  if( Status == PanExec::QNull ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize((uint32_t)(Status));
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleCancel(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN CANCEL Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  unsigned Idx = event->getSize();
  if( !PExec->RemoveEntry(Idx) ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Idx);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleReserve(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN RESERVE Request\n");
  if( PNic->IsReserved() ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  if( !PNic->SetToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleRevoke(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN REVOKE Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  ReadyForRevoke = true;

#if 0
  // write the completion
  uint64_t Addr = _PAN_COMPLETION_ADDR_;
  uint64_t Payload = 0x01ull;
  Mem->WriteMem(Addr,8,&Payload);

  PNic->RevokeToken();
#endif
  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleHalt(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN HALT Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
    return ;
  }
  Procs[HART]->Halt();
  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleResume(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN RESUME Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  bool error = false;
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( !Procs[i]->Resume() )
      error = true;
  }
  if( error )
    PANBuildFailedToken(event);
  else
    PANBuildRawSuccess(event);
}

void RevCPU::PANHandleReadReg(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN READREG Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
    return ;
  }

  unsigned Idx = (unsigned)(event->getAddr());
  uint64_t Value = 0x00ull;

  if( !Procs[HART]->DebugReadReg(Idx,&Value) ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent(getName());

  uint32_t Width = 0;
  if( Procs[HART]->DebugIsRV32() )
    Width = 4;
  else
    Width = 8;

  SCmd->setData(&Value,Width);
  SCmd->setSize(Width);
  SCmd->setSrc(address);

  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleWriteReg(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN WRITEREG Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
    return ;
  }

  unsigned Idx = (unsigned)(event->getAddr());
  uint64_t Value = 0x00ull;
  event->getData(&Value);

  if( !Procs[HART]->DebugWriteReg(Idx,Value) ){
    PANBuildFailedToken(event);
    return ;
  }

  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleSingleStep(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN SINGLESTEP Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
    return ;
  }

  uint64_t PC = Procs[HART]->GetPC();
  if( !Procs[HART]->SingleStepHart() ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent(getName());
  PANBuildBasicSuccess(event,SCmd);
  SCmd->setAddr(PC);
  SCmd->setSrc(address);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleSetFuture(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN SETFUTURE Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  if( Mem->SetFuture(event->getAddr()))
    PANBuildRawSuccess(event);
  else
    PANBuildFailedToken(event);
}

void RevCPU::PANHandleRevokeFuture(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN REVOKEFUTURE Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  if( Mem->RevokeFuture(event->getAddr()))
    PANBuildRawSuccess(event);
  else
    PANBuildFailedToken(event);
}

void RevCPU::PANHandleStatusFuture(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN STATUSFUTURE Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  bool IsFuture = Mem->StatusFuture(event->getAddr());
  panNicEvent *SCmd = new panNicEvent(getName());
  PANBuildBasicSuccess(event,SCmd);
  if( IsFuture )
    SCmd->setSize(0x01);
  else
    SCmd->setSize(0x00);
  SCmd->setSrc(address);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleBOTW(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN BOTW Request\n");
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  // commented out to prevent warnings
  // uint8_t VarArgs = event->getVarArgs();
  uint64_t Entry  = (uint64_t)(event->getOffset()) + Loader->GetSymbolAddr("_start");
  unsigned Idx    = 0;

  // marshall the varargs to the device
  // TODO

  if( !PExec->AddEntry(Entry,&Idx) ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent(getName());
  SCmd->setSize(Idx);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::handleHostPANMessage(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling Host PAN Message opcode=%d\n", (int)(event->getOpcode()));
  switch( event->getOpcode() ){
  case panNicEvent::Success:
    PANHandleSuccess(event);
    SuccessRecv->addData(1);
    break;
  case panNicEvent::Failed:
    PANHandleFailed(event);
    FailedRecv->addData(1);
    break;
  case panNicEvent::SyncGet:
    PANHandleSyncGet(event);
    SyncGetRecv->addData(1);
    break;
  case panNicEvent::SyncPut:
    PANHandleSyncPut(event);
    SyncPutRecv->addData(1);
    break;
  case panNicEvent::AsyncGet:
    PANHandleAsyncGet(event);
    AsyncGetRecv->addData(1);
    break;
  case panNicEvent::AsyncPut:
    PANHandleAsyncPut(event);
    AsyncPutRecv->addData(1);
    break;
  case panNicEvent::SyncStreamGet:
    PANHandleSyncStreamGet(event);
    SyncStreamGetRecv->addData(1);
    break;
  case panNicEvent::SyncStreamPut:
    PANHandleSyncStreamPut(event);
    SyncStreamPutRecv->addData(1);
    break;
  case panNicEvent::AsyncStreamGet:
    PANHandleAsyncStreamGet(event);
    AsyncStreamGetRecv->addData(1);
    break;
  case panNicEvent::AsyncStreamPut:
    PANHandleAsyncStreamPut(event);
    AsyncStreamPutRecv->addData(1);
    break;
  case panNicEvent::Exec:
  case panNicEvent::Status:
  case panNicEvent::Cancel:
  case panNicEvent::Reserve:
  case panNicEvent::Revoke:
  case panNicEvent::Halt:
  case panNicEvent::Resume:
  case panNicEvent::ReadReg:
  case panNicEvent::WriteReg:
  case panNicEvent::SingleStep:
  case panNicEvent::SetFuture:
  case panNicEvent::RevokeFuture:
  case panNicEvent::StatusFuture:
  case panNicEvent::BOTW:
  default:
    // host devices should never receive these commands
    output.fatal(CALL_INFO, -1,
                 "Error: host devices cannot handle %s commands; Opcode = %d; SrcID = %d\n",
                 event->getOpcodeStr().c_str(), event->getOpcode(), event->getSrc() );
    break;
  }
}

void RevCPU::handleNetPANMessage(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN Message opcode=%d; tag=%d\n",
                 (int)(event->getOpcode()),
                 (int)(event->getTag()));
  switch( event->getOpcode() ){
  case panNicEvent::SyncGet:
    PANHandleSyncGet(event);
    SyncGetRecv->addData(1);
    break;
  case panNicEvent::SyncPut:
    PANHandleSyncPut(event);
    SyncPutRecv->addData(1);
    break;
  case panNicEvent::AsyncGet:
    PANHandleAsyncGet(event);
    AsyncGetRecv->addData(1);
    break;
  case panNicEvent::AsyncPut:
    PANHandleAsyncPut(event);
    AsyncPutRecv->addData(1);
    break;
  case panNicEvent::SyncStreamGet:
    PANHandleSyncStreamGet(event);
    SyncStreamGetRecv->addData(1);
    break;
  case panNicEvent::SyncStreamPut:
    PANHandleSyncStreamPut(event);
    SyncStreamPutRecv->addData(1);
    break;
  case panNicEvent::AsyncStreamGet:
    PANHandleAsyncStreamGet(event);
    AsyncStreamGetRecv->addData(1);
    break;
  case panNicEvent::AsyncStreamPut:
    PANHandleAsyncStreamPut(event);
    AsyncStreamPutRecv->addData(1);
    break;
  case panNicEvent::Exec:
    PANHandleExec(event);
    ExecRecv->addData(1);
    break;
  case panNicEvent::Status:
    PANHandleStatus(event);
    StatusRecv->addData(1);
    break;
  case panNicEvent::Cancel:
    PANHandleCancel(event);
    CancelRecv->addData(1);
    break;
  case panNicEvent::Reserve:
    PANHandleReserve(event);
    ReserveRecv->addData(1);
    break;
  case panNicEvent::Revoke:
    PANHandleRevoke(event);
    RevokeRecv->addData(1);
    break;
  case panNicEvent::Halt:
    PANHandleHalt(event);
    HaltRecv->addData(1);
    break;
  case panNicEvent::Resume:
    PANHandleResume(event);
    ResumeRecv->addData(1);
    break;
  case panNicEvent::ReadReg:
    PANHandleReadReg(event);
    ReadRegRecv->addData(1);
    break;
  case panNicEvent::WriteReg:
    PANHandleWriteReg(event);
    WriteRegRecv->addData(1);
    break;
  case panNicEvent::SingleStep:
    PANHandleSingleStep(event);
    SingleStepRecv->addData(1);
    break;
  case panNicEvent::SetFuture:
    PANHandleSetFuture(event);
    SingleStepRecv->addData(1);
    break;
  case panNicEvent::RevokeFuture:
    PANHandleRevokeFuture(event);
    SetFutureRecv->addData(1);
    break;
  case panNicEvent::StatusFuture:
    PANHandleStatusFuture(event);
    StatusFutureRecv->addData(1);
    break;
  case panNicEvent::BOTW:
    PANHandleBOTW(event);
    BOTWRecv->addData(1);
    break;
  case panNicEvent::Success:
    PANHandleSuccess(event);
    SuccessRecv->addData(1);
    break;
  case panNicEvent::Failed:
    PANHandleFailed(event);
    FailedRecv->addData(1);
    break;
  default:
    // network devices should never receive these commands
    output.fatal(CALL_INFO, -1,
                 "Error: network devices cannot handle %s commands\n",
                 event->getOpcodeStr().c_str() );
    break;
  }
}

void RevCPU::registerSendCmd(panNicEvent *event){
  switch( event->getOpcode() ){
  case panNicEvent::SyncGet:
    SyncGetSend->addData(1);
    break;
  case panNicEvent::SyncPut:
    SyncPutSend->addData(1);
    break;
  case panNicEvent::AsyncGet:
    AsyncGetSend->addData(1);
    break;
  case panNicEvent::AsyncPut:
    AsyncPutSend->addData(1);
    break;
  case panNicEvent::SyncStreamGet:
    SyncStreamGetSend->addData(1);
    break;
  case panNicEvent::SyncStreamPut:
    SyncStreamPutSend->addData(1);
    break;
  case panNicEvent::AsyncStreamGet:
    AsyncStreamGetSend->addData(1);
    break;
  case panNicEvent::AsyncStreamPut:
    AsyncStreamPutSend->addData(1);
    break;
  case panNicEvent::Exec:
    ExecSend->addData(1);
    break;
  case panNicEvent::Status:
    StatusSend->addData(1);
    break;
  case panNicEvent::Cancel:
    CancelSend->addData(1);
    break;
  case panNicEvent::Reserve:
    ReserveSend->addData(1);
    break;
  case panNicEvent::Revoke:
    RevokeSend->addData(1);
    break;
  case panNicEvent::Halt:
    HaltSend->addData(1);
    break;
  case panNicEvent::Resume:
    ResumeSend->addData(1);
    break;
  case panNicEvent::ReadReg:
    ReadRegSend->addData(1);
    break;
  case panNicEvent::WriteReg:
    WriteRegSend->addData(1);
    break;
  case panNicEvent::SingleStep:
    SingleStepSend->addData(1);
    break;
  case panNicEvent::SetFuture:
    SingleStepSend->addData(1);
    break;
  case panNicEvent::RevokeFuture:
    SetFutureSend->addData(1);
    break;
  case panNicEvent::StatusFuture:
    StatusFutureSend->addData(1);
    break;
  case panNicEvent::BOTW:
    BOTWSend->addData(1);
    break;
  case panNicEvent::Success:
    SuccessSend->addData(1);
    break;
  case panNicEvent::Failed:
    FailedSend->addData(1);
    break;
  default:
    // network devices should never receive these commands
    output.fatal(CALL_INFO, -1,
                 "Error: no statistic for command; opcode =%d\n",
                 event->getOpcode() );
    break;
  }
}

bool RevCPU::sendPANMessage(){
  // check the mailbox for the next message
  if( SendMB.empty() )
    return true;

  output.verbose(CALL_INFO, 4, 0,
                 "Sending PAN message from %d to %d; Opc=%s; Tag=%u; Token=%" PRIu32 "; Size=%" PRIu32 "\n",
                 address, SendMB.front().second,
                 SendMB.front().first->getOpcodeStr().c_str(),
                 SendMB.front().first->getTag(),
                 SendMB.front().first->getToken(),
                 SendMB.front().first->getSize());
  PNic->send(SendMB.front().first,SendMB.front().second);

  uint8_t Opc = SendMB.front().first->getOpcode();
  if( (Opc != panNicEvent::Success) && (Opc != panNicEvent::Failed) ){
    TrackTags.push_back(std::make_pair(SendMB.front().first->getTag(),
                                       SendMB.front().second));
  }

  if( EnablePANStats )
    registerSendCmd(SendMB.front().first);

  // pop the message off the queue
  SendMB.pop();

  // handle the revocation
  if( ReadyForRevoke ){
    output.verbose(CALL_INFO, 4, 0, "Revoking token\n");
    // write the completion
    uint64_t Addr = _PAN_COMPLETION_ADDR_;
    uint64_t Payload = 0x01ull;
    Mem->WriteMem(0, Addr,8,&Payload);

    PNic->RevokeToken();
    ReadyForRevoke = false;
    RevokeHasArrived = true;
  }

  return true;
}

bool RevCPU::processPANZeroAddr(){
  if( ZeroRqst.empty() ){
    // nothing to do
    return true;
  }

  output.verbose(CALL_INFO, 5, 0, "Processing Zero Address Put Commands\n");

  // size_t XferSize = sizeof(PRTIME_XFER);
  PRTIME_XFER *XferPtr = (PRTIME_XFER *)(_PAN_XFER_BUF_ADDR_);
  uint8_t TmpValid = _PAN_ENTRY_INVALID_;
  char *TmpPtr = nullptr;
  uint32_t TmpSize = 0;

  for( unsigned i=0; i<_PAN_RDMA_MAX_ENTRIES_; i++ ){
    if( ZeroRqst.empty() ){
      break;
    }

    if( !Mem->ReadMem((uint64_t)(&XferPtr[i].Valid),
                      8,
                      &TmpValid) ){
      output.fatal(CALL_INFO, -1,
                   "Error: Could not read valid bit for zero address data insertion; Addr=0x%" PRIx64 "\n",
                   (uint64_t)(&XferPtr[i].Valid));
    }

    if( TmpValid == _PAN_ENTRY_INVALID_ ){
      // found an open slot
      TmpValid = _PAN_ENTRY_VALID_;
      TmpSize = ZeroRqst.front().first;
      TmpPtr  = ZeroRqst.front().second;

      Mem->Write(0, (uint64_t)(&XferPtr[i].Valid), TmpValid);
      Mem->WriteMem(0, (uint64_t)(&XferPtr[i].Buffer[0]), TmpSize, TmpPtr);

      ZeroRqst.pop();
      delete[] TmpPtr;
    }
  }

  return true;
}

bool RevCPU::processPANMemRead(){
  // walk the entire vector and decrement all the counters
  // if we have a counter decrement to '0', then process the read request
  // send responses as necessary

  // decrement all the counts
  int i = 0;
  for( auto &it : ReadQueue ){
    if( std::get<2>(it) != 0 )
      std::get<2>(it)--;
    i++;
  }

  // walk all the nodes and see which requests need to be flushed
  for( auto it=ReadQueue.begin(); it != ReadQueue.end(); ++it ){
    if( std::get<2>(*it) == 0 ){
      // process this read request
      uint8_t tmp_tag = std::get<0>(*it);
      uint32_t tmp_size = std::get<1>(*it);
      int tmp_src = std::get<3>(*it);
      uint64_t tmp_addr = std::get<4>(*it);

      // -- setup the response
      panNicEvent *SCmd = new panNicEvent(getName());
      uint64_t *Data = new uint64_t [SCmd->getNumBlocks(tmp_size)];
      if( !Mem->ReadMem( tmp_addr,
                         (size_t)(tmp_size),
                         Data)){
        // build a failed response
        SCmd->buildFailed(PNic->GetToken(),tmp_tag);
        SCmd->setSrc(address);
        SendMB.push(std::make_pair(SCmd,tmp_src));
      }else{
        // build a successful response
        SCmd->buildSuccess(PNic->GetToken(),tmp_tag);
        SCmd->setSize(tmp_size);
        SCmd->setData(Data,tmp_size);
        SCmd->setSrc(address);
        SendMB.push(std::make_pair(SCmd,tmp_src));
      }
      delete[] Data;
    }// else do nothing
  }

  // delete the completed nodes
  unsigned count = ReadQueue.size();
  for( unsigned i=0; i<count;){
    if( std::get<2>(ReadQueue[i]) == 0 ){
      ReadQueue.erase(ReadQueue.begin()+i);
      --count;
    }else{
      ++i;
    }
  }

  return true;
}

bool RevCPU::PANConvertRDMAtoEvent(uint64_t Addr, panNicEvent *event){

  // Stage 1: read the first 64 bits and decode it
  uint64_t Payload  = Mem->ReadU64(Addr);
  uint8_t Tag       = 0x0;
  uint8_t Opcode    = 0x0;
  uint8_t VarArgs   = 0x0;
  uint32_t Size     = 0x0;
  uint32_t Token    = 0x0;
  uint32_t Offset   = 0x0;
  uint64_t CmdAddr  = 0x00ull;
  uint64_t DataAddr = 0x00ull;
  uint64_t TmpData  = 0x00ull;
  uint64_t *Data    = nullptr;


  Token  = (uint32_t)( Payload & 0xffffffff );
  Tag    = (uint8_t)( (Payload >> 32) & 0b11111111 );
  Opcode = (uint8_t)( (Payload >> 40) & 0b11);

  switch( Opcode ){
  case 0b00:
    // Base packet
    Opcode = (uint8_t)( (Payload >> 40) & 0b11111111);
    Size   = (uint32_t)((Payload >> 48) & 0b1111111111111111);
    break;
  case 0b01:
    // Streaming packet
    Opcode = (uint8_t)( (Payload >> 40) & 0b1111);
    Size   = (uint32_t)((Payload >> 44) & 0b11111111111111111111);
    break;
  case 0b11:
    // BOTW packet
    VarArgs = (uint8_t)((Payload >> 42) & 0b1111);
    Offset  = (uint32_t)((Payload >> 46) & 0b111111111111111111);
    break;
  case 0b10:
  default:
    output.fatal(CALL_INFO, -1,
                "Error: Bad opcode type from RDMA Mailbox Command: Opc=%d\n", Opcode);
    break;
  }

  output.verbose(CALL_INFO, 5, 0,
                 "Coverting RDMA Mailbox Entry to Event: Payload = %" PRIu64 "; Opcode=%d\n",
                 Payload, Opcode);

  // Stage 2: use the opcode the read and encode the remainder of the data
  switch(Opcode){
  case panNicEvent::SyncGet:
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    if( !event->buildSyncGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SyncGet; Tag=%d\n",Tag);
    TrackGets.push_back(std::make_tuple(Tag,DataAddr,Size));
    break;
  case panNicEvent::SyncPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    //if( !Mem->ReadMem(Addr+16,Size,Data) ){
    if( !Mem->ReadMem(DataAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                   "Error: could not retrieve data for RDMA SyncPut; Tag=%d\n",Tag);
    }
    if( !event->buildSyncPut(Token,Tag,CmdAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SyncPut; Tag=%d\n",Tag);
    }
    delete[] Data;
    break;
  case panNicEvent::AsyncGet:
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    if( !event->buildAsyncGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA AsyncGet; Tag=%d\n",Tag);
    TrackGets.push_back(std::make_tuple(Tag,DataAddr,Size));
    break;
  case panNicEvent::AsyncPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    //if( !Mem->ReadMem(Addr+16,Size,Data) ){
    if( !Mem->ReadMem(DataAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                   "Error: could not retrieve data for RDMA AsyncPut; Tag=%d\n",Tag);
    }
    if( !event->buildAsyncPut(Token,Tag,CmdAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA AsyncPut; Tag=%d\n",Tag);
    }
    delete[] Data;
    break;
  case panNicEvent::SyncStreamGet:
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    if( !event->buildSyncStreamGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SyncStreamGet; Tag=%d\n",Tag);
    TrackGets.push_back(std::make_tuple(Tag,DataAddr,Size));
    break;
  case panNicEvent::SyncStreamPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    //if( !Mem->ReadMem(Addr+8,Size,Data) ){
    if( !Mem->ReadMem(DataAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                   "Error: could not retrieve data for RDMA SyncStreamPut; Tag=%d\n",Tag);
    }
    if( !event->buildSyncStreamPut(Token,Tag,CmdAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SyncStreamPut; Tag=%d\n",Tag);
    }
    delete[] Data;
    break;
  case panNicEvent::AsyncStreamGet:
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    if( !event->buildAsyncStreamGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA AsyncStreamGet; Tag=%d\n",Tag);
    TrackGets.push_back(std::make_tuple(Tag,DataAddr,Size));
    break;
  case panNicEvent::AsyncStreamPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    DataAddr = Mem->ReadU64(Addr+16);
    //if( !Mem->ReadMem(Addr+8,Size,Data) ){
    if( !Mem->ReadMem(DataAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                   "Error: could not retrieve data for RDMA AsyncStreamPut; Tag=%d\n",Tag);
    }
    if( !event->buildAsyncStreamPut(Token,Tag,CmdAddr,Size,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA AsyncStreamPut; Tag=%d\n",Tag);
    }
    delete[] Data;
    break;
  case panNicEvent::Exec:
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !event->buildExec(Token,Tag,CmdAddr) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Exec; Tag=%d\n",Tag);
    break;
  case panNicEvent::Status:
    if( !event->buildStatus(Token,Tag, (uint16_t)(Size)) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Status; Tag=%d\n",Tag);
    break;
  case panNicEvent::Cancel:
    if( !event->buildCancel(Token,Tag, (uint16_t)(Size)) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Cancel; Tag=%d\n",Tag);
    break;
  case panNicEvent::Reserve:
    if( !event->buildReserve(Token,Tag) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Reserve; Tag=%d\n",Tag);
    break;
  case panNicEvent::Revoke:
    if( !event->buildRevoke(Token,Tag) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Revoke; Tag=%d\n",Tag);
    CmdAddr = _PAN_COMPLETION_ADDR_;
    TmpData = 0x01;
    Mem->WriteMem(0, CmdAddr,8,&TmpData);
    break;
  case panNicEvent::Halt:
    if( !event->buildHalt(Token,Tag,(uint16_t)(Size)) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Halt; Tag=%d\n",Tag);
    break;
  case panNicEvent::Resume:
    if( !event->buildResume(Token,Tag) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Resume; Tag=%d\n",Tag);
    break;
  case panNicEvent::ReadReg:
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !event->buildReadReg(Token,Tag,Size,CmdAddr) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA ReadReg; Tag=%d\n",Tag);
    break;
  case panNicEvent::WriteReg:
    CmdAddr = Mem->ReadU64(Addr+8);
    TmpData = Mem->ReadU64(Addr+16);
    if( !event->buildWriteReg(Token,Tag,Size,CmdAddr,&TmpData) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA WriteReg; Tag=%d\n",Tag);
    break;
  case panNicEvent::SingleStep:
    if( !event->buildSingleStep(Token,Tag,(uint16_t)(Size)) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SingleStep; Tag=%d\n",Tag);
    break;
  case panNicEvent::SetFuture:
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !event->buildSetFuture(Token,Tag,CmdAddr) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SetFuture; Tag=%d\n",Tag);
    break;
  case panNicEvent::RevokeFuture:
    if( !event->buildRevokeFuture(Token,Tag,CmdAddr) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA RevokeFuture; Tag=%d\n",Tag);
    break;
  case panNicEvent::StatusFuture:
    if( !event->buildStatusFuture(Token,Tag,CmdAddr) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA StatusFuture; Tag=%d\n",Tag);
    break;
  case panNicEvent::Success:
    if( !event->buildSuccess(Token,Tag) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Success; Tag=%d\n",Tag);
    break;
  case panNicEvent::Failed:
    if( !event->buildFailed(Token,Tag) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA Failed; Tag=%d\n",Tag);
    break;
  case panNicEvent::BOTW:
    Data = new uint64_t [VarArgs];
    if( !Mem->ReadMem(Addr+8,VarArgs*8,Data) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                   "Error: could not retrieve VarArgs data for RDMA BOTW; Tag=%d\n",Tag);
    }
    if( !event->buildBOTW(Token,Tag,VarArgs,Data,Offset) ){
      delete[] Data;
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA BOTW; Tag=%d\n",Tag);
    }
    delete[] Data;
    break;
  default:
    output.fatal(CALL_INFO, -1,
                 "Error: could not encode RDMA opcode=%d\n",Opcode);
    break;
  }

  // Stage n: set the src
  event->setSrc(address);

  return true;
}

bool RevCPU::PANProcessRDMAMailbox(){
  output.verbose(CALL_INFO, 8, 0, "Handling PAN RDMA Mailbox\n");
  // check to see if there is space in the outgoing message queue
  if( SendMB.size() == 255 )
    return true;

  // there is space in the outgoing message queue, walk the messages
  bool done = false;
  unsigned sent = 0;
  unsigned iter = 0;
  uint64_t Addr = PrevAddr;
  uint64_t Payload[3];

  while( !done ){

    //
    // retrive the 'iter' 24-byte mailbox payload
    // each payload is configured as follows:
    //      ADDR + 16          ADDR + 8             ADDR
    // [  EVENT POINTER  ][      DEST       ][      VALID      ]
    //
    // If `Valid` == _PAN_ENTRY_VALID_; The entry is valid, consume it
    // Else, continue
    //

    // Stage 1: Read the memory
    Mem->ReadMem(Addr,24,&Payload[0]);

    // Stage 2: Interrogate the payload
    if( Payload[0] == _PAN_ENTRY_VALID_ ){
      // found a valid payload, process it
      output.verbose(CALL_INFO, 8, 0, "Processing RDMA Mailbox Command; Entry=%u\n", iter);

      // Stage 2.a: Convert the buffer into an event
      panNicEvent *FEvent = new panNicEvent(getName());

      if( !PANConvertRDMAtoEvent(Payload[2],FEvent) )
        output.fatal(CALL_INFO, -1,
                     "Error: could not convert RDMA command to event from address=0x%" PRIx64 "\n",
                     Payload[1]);

      // Stage 2.b: Insert the event into the send queue
      SendMB.push(std::make_pair(FEvent,(int)(Payload[1])));

      // Stage 2.c: mark the payload as being completed
      Payload[0] = _PAN_ENTRY_INJECTED_;
      Mem->WriteMem(0, Addr,8,&Payload[0]);

      // Stage 2.d: increment the message counter
      sent++;
    }// else invalid or injected

    // Stage 3: Update the counters & the Address
    iter++;
    Addr += 24;
    Payload[0] = 0x00ull;
    Payload[1] = 0x00ull;
    Payload[2] = 0x00ull;

    if( Addr == (_PAN_RDMA_MAILBOX_ + (24*_PAN_RDMA_MAX_ENTRIES_)) ){
      Addr = _PAN_RDMA_MAILBOX_;
    }

    if( iter == _PAN_RDMA_MAX_ENTRIES_ ){
      PrevAddr = Addr;
      done = true;
    }else if( sent == RDMAPerCycle ){
      PrevAddr = Addr;
      done = true;
    }else if( SendMB.size() == 255 ){
      PrevAddr = Addr;
      done = true;
    }
  }

  return true;
}

uint8_t RevCPU::createTag(){
  uint8_t rtn = 0;
  if( PrivTag == 0b11111111 ){
    rtn = 0b00000000;
    PrivTag = 0b00000001;
    return 0b00000000;
  }else{
    rtn = PrivTag;
    PrivTag +=1;
  }
  return rtn;
}

void RevCPU::ExecPANTest(){

  // wait for the previous test to finish
  if( !TrackTags.empty() )
    return ;

  int dest = 1;
  uint64_t BASE = 0x00000080ull;
  uint64_t Buf = 0x00ull;
  panNicEvent *TEvent = nullptr;

  switch( testStage ){
  case 0:
    // Token reservation
    output.verbose(CALL_INFO, 4, 0, "Executing RESERVE test\n");
    TEvent = new panNicEvent(getName());
    LToken = 0xfeedbeef;
    TEvent->setSrc(address);
    if( !TEvent->buildReserve(LToken,createTag()) )
      output.fatal(CALL_INFO, -1, "Error: could not create PAN RESERVE command\n");
    SendMB.push(std::make_pair(TEvent,dest));
    testStage++;
    break;
  case 1:
    // Sync_Put test
    output.verbose(CALL_INFO, 4, 0, "Executing SYNC_PUT test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildSyncPut(LToken,
                                createTag(),
                                BASE+(uint64_t)(i*8),
                                8,
                                &Buf) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN SYNC_PUT command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 2:
    // Sync_Get test
    output.verbose(CALL_INFO, 4, 0, "Executing SYNC_GET test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildSyncGet(LToken,
                                createTag(),
                                BASE+(uint64_t)(i*8),
                                8 ) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN SYNC_GET command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 3:
    // Async_Put test
    output.verbose(CALL_INFO, 4, 0, "Executing ASYNC_PUT test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildAsyncPut(LToken,
                                 createTag(),
                                 BASE+(uint64_t)(i*8),
                                 8,
                                 &Buf) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN ASYNC_PUT command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 4:
    // Async_Get test
    output.verbose(CALL_INFO, 4, 0, "Executing ASYNC_GET test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildAsyncGet(LToken,
                                createTag(),
                                BASE+(uint64_t)(i*8),
                                8 ) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN ASYNC_GET command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 5:
    // Sync_Stream_Put test
    output.verbose(CALL_INFO, 4, 0, "Executing SYNC_STREAM_PUT test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildSyncStreamPut(LToken,
                                      createTag(),
                                      BASE+(uint64_t)(i*8),
                                      8,
                                      &Buf) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN SYNC_STREAM_PUT command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 6:
    // Sync_Stream_Get test
    output.verbose(CALL_INFO, 4, 0, "Executing SYNC_STREAM_GET test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildSyncStreamGet(LToken,
                                createTag(),
                                BASE+(uint64_t)(i*8),
                                8 ) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN SYNC_STREAM_GET command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 7:
    // ASync_Stream_Put test
    output.verbose(CALL_INFO, 4, 0, "Executing ASYNC_STREAM_PUT test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildAsyncStreamPut(LToken,
                                       createTag(),
                                       BASE+(uint64_t)(i*8),
                                       8,
                                       &Buf) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN ASYNC_STREAM_PUT command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 8:
    // ASync_Stream_Get test
    output.verbose(CALL_INFO, 4, 0, "Executing ASYNC_STREAM_GET test\n");
    for( unsigned i=0; i<testIters; i++ ){
      // create a new event
      TEvent = new panNicEvent(getName());  // new event to send
      TEvent->setSrc(address);
      Buf = 0xdeadbeef;            // buffer to send

      // populate it
      if( !TEvent->buildAsyncStreamGet(LToken,
                                createTag(),
                                BASE+(uint64_t)(i*8),
                                8 ) )
        output.fatal(CALL_INFO, -1, "Error: could not create PAN ASYNC_STREAM_GET command\n" );

      // send it
      SendMB.push(std::make_pair(TEvent,dest));
    }
    testStage++;
    break;
  case 9:
    // write the completion command
    testStage++;
    output.verbose(CALL_INFO, 4, 0, "Sending destination completion command\n");
    TEvent = new panNicEvent(getName());
    TEvent->setSrc(address);
    Buf = 0xdeadbeef;
    if( !TEvent->buildSyncPut(LToken,
                              createTag(),
                              (uint64_t)(_PAN_COMPLETION_ADDR_),
                              8,
                              &Buf) )
      output.fatal(CALL_INFO, -1, "Error: could not send completion command to destination\n" );
    SendMB.push(std::make_pair(TEvent,dest));

    // write the completion command to our local CPU
    Mem->Write(0, (uint64_t)(_PAN_COMPLETION_ADDR_), uint64_t{0xdeadbeef});
    break;
  case 10:
    // revoke reservation
    output.verbose(CALL_INFO, 4, 0, "Executing REVOKE test\n");
    TEvent = new panNicEvent(getName());
    TEvent->setSrc(address);
    if( !TEvent->buildRevoke(LToken,createTag()) )
      output.fatal(CALL_INFO, -1, "Error: could not create PAN REVOKE command\n");
    SendMB.push(std::make_pair(TEvent,dest));
    testStage++;
    break;
  default:
    // do nothing
    break;
  }
}

bool RevCPU::clockTickPANTest( SST::Cycle_t currentCycle ){
  bool rtn = true;
  output.verbose(CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", currentCycle);

  // run test harness
  ExecPANTest();

  // Execute each enabled core
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] ){
      if( !Procs[i]->ClockTick(currentCycle) ){
        Enabled[i] = false;
      }
    }
  }

  // inject messages
  for( unsigned i=0; i< msgPerCycle; i++ ){
    // check the mailbox for messages to inject
    if( !sendPANMessage() )
      output.fatal(CALL_INFO, -1, "Error: could not send PAN command message\n" );
  }

  // check to see if all the processors are completed
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] )
      rtn = false;
  }

  // check to see if we have outstanding network messages and whether the tests are complete
  if( (!SendMB.empty() || !TrackTags.empty()) &&
      (testStage < _MAX_PAN_TEST_) ){
    rtn = false;
  }

  // if its time to return, end the sim
  if( rtn )
    primaryComponentOKToEndSim();

  return rtn;
}

void RevCPU::HandleCrackFault(SST::Cycle_t currentCycle){
  output.verbose(CALL_INFO, 4, 0, "FAULT: Crack fault injected at cycle: %" PRIu64 "\n",
                 currentCycle);

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ){
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, numCores-1); // define the range
    Core = distr(gen);
  }

  Procs[Core]->HandleCrackFault(fault_width);
}

void RevCPU::HandleMemFault(SST::Cycle_t currentCycle){
  output.verbose(CALL_INFO, 4, 0, "FAULT: Memory fault injected at cycle: %" PRIu64 "\n",
                 currentCycle);
  Mem->HandleMemFault(fault_width);
}

void RevCPU::HandleRegFault(SST::Cycle_t currentCycle){
  output.verbose(CALL_INFO, 4, 0, "FAULT: Register fault injected at cycle: %" PRIu64 "\n",
                currentCycle);

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ){
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, numCores-1); // define the range
    Core = distr(gen);
  }

  Procs[Core]->HandleRegFault(fault_width);
}

void RevCPU::HandleALUFault(SST::Cycle_t currentCycle){
  output.verbose(CALL_INFO, 4, 0, "FAULT: ALU fault injected at cycle: %" PRIu64 "\n",
                 currentCycle);

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ){
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, numCores-1); // define the range
    Core = distr(gen);
  }

  Procs[Core]->HandleALUFault(fault_width);
}

void RevCPU::HandleFaultInjection(SST::Cycle_t currentCycle){
  // build up a vector of faults
  // then randomly determine which one to inject
  std::vector<std::string> myfaults;
  if( EnableCrackFaults ){
    myfaults.push_back("crack");
  }
  if( EnableMemFaults ){
    myfaults.push_back("mem");
  }
  if( EnableRegFaults ){
    myfaults.push_back("reg");
  }
  if( EnableALUFaults ){
    myfaults.push_back("alu");
  }

  if( myfaults.size() == 0 ){
    output.fatal(CALL_INFO, -1,
                 "Error: no faults enabled; add a fault vector in the 'faults' param\n" );
  }

  unsigned selector = 0;
  if( myfaults.size() != 1 ){
    selector = RevRand(0, int(myfaults.size())-1);
  }

  // handle the selected fault
  if( myfaults[selector] == "crack" ){
    HandleCrackFault(currentCycle);
  }else if( myfaults[selector] == "mem" ){
    HandleMemFault(currentCycle);
  }else if( myfaults[selector] == "reg" ){
    HandleRegFault(currentCycle);
  }else if( myfaults[selector] == "alu" ){
    HandleALUFault(currentCycle);
  }else{
    output.fatal(CALL_INFO, -1, "Error: erroneous fault selection\n" );
  }
}

void RevCPU::UpdateCoreStatistics(uint16_t coreNum){
  RevProc::RevProcStats stats = Procs[coreNum]->GetStats();
  TotalCycles[coreNum]->addData(stats.totalCycles);
  CyclesWithIssue[coreNum]->addData(stats.cyclesBusy);
  FloatsRead[coreNum]->addData(stats.memStats.floatsRead);
  FloatsWritten[coreNum]->addData(stats.memStats.floatsWritten);
  DoublesRead[coreNum]->addData(stats.memStats.doublesRead);
  DoublesWritten[coreNum]->addData(stats.memStats.doublesWritten);
  BytesRead[coreNum]->addData(stats.memStats.bytesRead);
  BytesWritten[coreNum]->addData(stats.memStats.bytesWritten);
  FloatsExec[coreNum]->addData(stats.floatsExec);
  TLBHitsPerCore[coreNum]->addData(stats.memStats.TLBHits);
  TLBMissesPerCore[coreNum]->addData(stats.memStats.TLBMisses);
}

bool RevCPU::clockTick( SST::Cycle_t currentCycle ){
  bool rtn = true;

  output.verbose(CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", currentCycle);

  // Execute each enabled core
  for( unsigned i=0; i<Procs.size(); i++ ){
   double Util = Procs[i]->GetUtilization();
    if( Util > 0.0 ){
      output.verbose(CALL_INFO, 10, 0, "Core %d utilization: %.2f%%\n", i, Util);
    }
    // Check if we have room to schedule another thread
    if( Util < 100 && ThreadQueue.size() ){
      output.verbose(CALL_INFO, 10, 0, "Core %d utilization: %.2f%%\n", i, Util);
      // We can schedule another thread
      // Check if we have any threads to schedule
      if( ThreadQueue.size() ){
        // Add to this proc's thread list
        Threads.at(ThreadQueue.front())->SetState(ThreadState::RUNNING);
        AssignedThreads.at(i).emplace_back(Threads.at(ThreadQueue.front()));
        output.verbose(CALL_INFO, 6, 1, "Assigning Thread %u to Core %u\n", ThreadQueue.front(), i);
        // Remove from thread queue
        ThreadQueue.erase(ThreadQueue.begin());
        // If this Proc was previously disabled, enable it
        // if( !Enabled[i] ){ Enabled[i] = true; }
        Enabled[i] = true;
      }
    } // Utilization is 100%, so change nothing

    if( Enabled[i] ){
      if( Procs[i]->GetUtilization() == 0.0 ){
        Enabled[i] = false;
        std::cout << "Disabled Proc " << i << std::endl;
        break;
      }
      // std::cout << "Proc " << i << " is enabled" << std::endl;
      if( !Procs[i]->ClockTick(currentCycle) ){
        if(EnableCoProc && !CoProcs.empty()){
         CoProcs[i]->Teardown();
        }
        UpdateCoreStatistics(i);
        Enabled[i] = false;
      output.verbose(CALL_INFO, 5, 0, "Closing Processor %u at Cycle: %" PRIu64 "\n",
                     i, currentCycle);
      }
      if(EnableCoProc && !CoProcs[i]->ClockTick(currentCycle)){
        output.verbose(CALL_INFO, 5, 0, "Closing Co-Processor %u at Cycle: %" PRIu64 "\n",
                       i, currentCycle);
      }
    }
    // Handle any thread state changes for this core
    std::bitset<_REV_HART_COUNT_>& Changes = Procs[i]->GetThreadStateChanges();

    if( Changes.any() ){
      for( unsigned j=0; j<Changes.size(); j++ ){
        if( Changes[j] ){
          std::shared_ptr<RevThread>& Thread = AssignedThreads.at(i).at(j);
          switch ( Thread->GetState() ) {
            case ThreadState::DONE:
              output.verbose(CALL_INFO, 8, 0, "Thread %u on Core %u is DONE\n", Thread->GetThreadID(), i);
              CompletedThreads.emplace(Thread->GetThreadID());
              // std::cout << "Made it here" << std::endl;
              AssignedThreads.at(i).erase(AssignedThreads.at(i).begin()+j);
              // std::cout << "Made it there" << std::endl;
              // Changes.flip(j);
              break;
            case ThreadState::BLOCKED:
              output.verbose(CALL_INFO, 8, 0, "Thread %u on Core %u is BLOCKED\n", Thread->GetThreadID(), i);
              if( ThreadCanProceed(Thread->GetThreadID()) ){
                output.verbose(CALL_INFO, 8, 0, "Thread %u on Core %u was waiting on thread %u which has already completed so it can proceed\n",
                               Thread->GetThreadID(), i, Thread->GetWaitingToJoinTID());
                // Continue executing thread on same Core
                Thread->SetState(ThreadState::RUNNING);
              }
              else {
                output.verbose(CALL_INFO, 8, 0, "Thread %u on Core %u was waiting on thread %u which has not yet completed so it remains blocked\n",
                               Thread->GetThreadID(), i, Thread->GetWaitingToJoinTID());
                Thread->SetState(ThreadState::BLOCKED);
                BlockedThreads.emplace(Thread->GetThreadID());

                AssignedThreads.at(i).erase(AssignedThreads.at(i).begin()+j);
              }
              // Changes.flip(j);
              break;
            case ThreadState::START: // Should never happen
              output.fatal(CALL_INFO, 99, "Error: Thread %u on Core %u is assigned but is in START state... This is a bug\n",
                           Thread->GetThreadID(), i);
              break;
            case ThreadState::RUNNING:
              output.verbose(CALL_INFO, 11, 0, "Thread %u on Core %u is RUNNING\n", Thread->GetThreadID(), i);
              break;
            case ThreadState::READY: // If this happens we are not setting state when assigning thread somewhere
              output.fatal(CALL_INFO, 99, "Error: Thread %u on Core %u is assigned but is in START state... This is a bug\n",
                           Thread->GetThreadID(), i);
              break;
            default: // Should DEFINITELY never happen
              output.fatal(CALL_INFO, 99, "Error: Thread %u on Core %u is in an unknown state... This is a bug\n",
                           Thread->GetThreadID(), i);
              break;
          }
          Changes[j] = false;
        }
        // This thread didn't experience a state change
        else {
          continue;
        }
      }
    }
    // Check for new threads
    if( !Procs[i]->GetNewThreadInfo().empty() ){
      output.verbose(CALL_INFO, 8, 0, "Core %d has new threads\n", i);
      // There are new thread(s) to create
      // auto NewThreadInfo = Procs[i]->GetNewThreadInfo();
      // std::cout << "NewThreadInfo.size() = " << NewThreadInfo.size() << std::endl;
      for( unsigned j=0; j<Procs[i]->GetNewThreadInfo().size(); j++ ){
        auto NewThread = Procs[i]->GetNewThreadInfo().front();
        Procs[i]->GetNewThreadInfo().pop();
        InitThread(NewThread);
      }
    }
    if( Procs[i]->GetUtilization() == 0 ){ Enabled[i] = false; }
  }
  // Clock the PAN network transport module
  if( EnablePAN ){

    if( EnableRDMAMBox ){
      // process the incoming mailbox messages from our local cores
      if( !PANProcessRDMAMailbox() )
        output.fatal(CALL_INFO, -1, "Error: failed to process the PAN RDMA mailbox\n");
    }

    for( unsigned i=0; i< msgPerCycle; i++ ){
      // check the mailbox for messages to inject
      if( !sendPANMessage() )
        output.fatal(CALL_INFO, -1, "Error: could not send PAN command message\n" );
    }

    // process the read queue
    if( !processPANMemRead() )
      output.fatal(CALL_INFO, -1, "Error: failed to process the PAN memory read queue\n" );

    // process the zero address put queue
    if( !processPANZeroAddr() )
      output.fatal(CALL_INFO, -1, "Error: failed to process the PAN zero address put queue\n" );
  }

  // check to see if we need to inject a fault
  if( EnableFaults ){
    if( FaultCntr == 0 ){
      // inject a fault
      HandleFaultInjection(currentCycle);

      // reset the fault counter
      FaultCntr = fault_width;
    }else{
      FaultCntr--;
    }
  }

  // check to see if all the processors are completed
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] ){
      rtn = false;
    }
  }

  // If all Procs are disabled (ie. rtn == false at this point)
  // check to see if there are threads to assign
  if( ThreadQueue.size() ){
    rtn = false;
  } else if ( BlockedThreads.size() ){
    if( ThreadCanProceed(*BlockedThreads.begin()) ){
      Threads.at(*(BlockedThreads.begin()))->SetState(ThreadState::READY);
      Threads.at(*(BlockedThreads.begin()))->SetWaitingToJoinTID(__INVALID_TID__);
      ThreadQueue.emplace_back(*BlockedThreads.begin());
      BlockedThreads.erase(BlockedThreads.begin());
    }
    rtn = false;
  }

  // check to see if the network has any outstanding messages: fixme
  if( !SendMB.empty() || !TrackTags.empty() || !ZeroRqst.empty() || !RevokeHasArrived ){
#ifdef _REV_DEBUG_
    if( RevokeHasArrived && !PNic->IsHost() ){
      if( !SendMB.empty() ){
        output.verbose(CALL_INFO,5,0,"SendMB not empty\n");
      }
      if( !ZeroRqst.empty() ){
        output.verbose(CALL_INFO,5,0,"ZeroRqst not empty\n");
      }
      if( !TrackTags.empty() ){
        output.verbose(CALL_INFO, 5, 0, "TrackTags not empty: %zu\n", TrackTags.size());
      }
    }
#endif
    rtn = false;
  }

  if( rtn && CompletedThreads.size() ){
    for( unsigned i=0; i<numCores; i++ ){
      Procs[i]->EndExecution();
    }
    primaryComponentOKToEndSim();
    output.verbose(CALL_INFO, 5, 0, "OK to end sim at cycle: %" PRIu64 "\n", currentCycle);
  } else {
    rtn = false;
  }

  return rtn;
}

///==== Thread Management Functions ====///
void RevCPU::InitThread(std::shared_ptr<RevThread> ThreadToInit){
  uint32_t TID = ThreadToInit->GetThreadID();
  // Check if this ThreadID has already been assigned... if so... something has gone horribly wrong
  // print out all threads
  auto it = Threads.find(TID);
  if( it != Threads.end() && it->second->GetState() != ThreadState::START ){
    std::cout << *Threads.at(TID) << std::endl;
    output.fatal(CALL_INFO, 99, "Error: ThreadID %d has already been assigned... this is a bug.\n", TID);
  }
  output.verbose(CALL_INFO, 4, 0, "Initializing Thread %d\n", TID);
  ThreadToInit->SetState(ThreadState::READY);
  Threads.emplace(ThreadToInit->GetThreadID(), ThreadToInit);
  ThreadQueue.emplace_back(ThreadToInit->GetThreadID());
}

// @brief Check if any BlockedThreads have had their counterpart complete execution
// if so, move its TID to the ThreadQueue
bool RevCPU::ThreadCanProceed(uint32_t TID){
  bool rtn = false;
  // Get the thread's waiting to join TID
  uint32_t WaitingOnTID = (Threads.at(TID))->GetWaitingToJoinTID();
  // If
  if( WaitingOnTID != __INVALID_TID__ ){
    // Check if WaitingOnTID has completed... if so, return = true, else return false
    std::cout << "Thread " << TID << " is waiting on Thread " << std::dec << WaitingOnTID << std::endl;
    rtn = ( CompletedThreads.find(WaitingOnTID) != CompletedThreads.end() ) ? true : false;
  }
  else if ( WaitingOnTID == __INVALID_TID__ ){
    rtn = true;
  }

  if( rtn ){
    std::cout << "Thread " << TID << " was waiting on Thread " << std::dec << WaitingOnTID << std::endl;
  }
  return rtn;
}

// Check if any BlockedThreads have had their counterpart complete execution
// if so, move its TID to the ThreadQueue
void RevCPU::CheckBlockedThreads(){
  for( auto ThreadID : BlockedThreads ){
    if( ThreadCanProceed(ThreadID) ){
      // Mark thread as ready (no longer blocked)
      Threads.at(ThreadID)->SetState(ThreadState::READY);
      // Remove the waiting to join TID
      Threads.at(ThreadID)->SetWaitingToJoinTID(__INVALID_TID__);
      // Add the thread to the ThreadQueue
      ThreadQueue.emplace_back(ThreadID);
      // Remove the thread from the BlockedThreads list
      BlockedThreads.erase(ThreadID);
    } else {
      continue;
    }
  }
  return;
}

// EOF
//
//
//
// ---------------- OLD CLOCKTICK
/// @brief Clock tick function for the RevCPU
/// @param currentCycle The current cycle of the simulation
// bool RevCPU::clockTick( SST::Cycle_t currentCycle ){
//   bool rtn = true;

//   output.verbose(CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", static_cast<uint64_t>(currentCycle));

//   // Execute each enabled core
//   for( unsigned i=0; i<Procs.size(); i++ ){
//     if( Procs[i]->GetUtilization() < 100 && ThreadQueue.size() ){
//       AssignedThreads.at(i).emplace_back(Threads.at(ThreadQueue.front()));
//       Threads.at(ThreadQueue.front())->SetState(ThreadState::RUNNING);
//       ThreadQueue.erase(ThreadQueue.begin());
//       if( !Enabled[i] ){ Enabled[i] = true;}
//     }
//     // If the Proc is enabled, keep executing
//     if( Enabled[i] ){
//       // if( !AssignedThreads.at(i).size() ){
//         // if there are no assigned threads and the Proc is enabled, disable it
//         // Enabled[i] = false;
//         // continue;
//       // }
//       // if ClockTick returns false, shut the Proc down
//       if( !Procs[i]->ClockTick(currentCycle) ){
//         if(EnableCoProc && !CoProcs.empty()){
//           CoProcs[i]->Teardown();
//         }
//         UpdateCoreStatistics(i);
//         Enabled[i] = false;
//         output.verbose(CALL_INFO, 5, 0, "Closing Processor %d at Cycle: %" PRIu64 "\n",
//                      i, static_cast<uint64_t>(currentCycle));
//       }
//       // FIXME: Do this a different way... but check if any of the AssignedThreads have done state
//       for( unsigned j=0; j<AssignedThreads.at(i).size(); j++ ){
//         std::shared_ptr<RevThread> Thread = AssignedThreads.at(i).at(j);
//         switch (Thread->GetState()) {
//           case ThreadState::DONE:{
//             // Need to do a check for any threads that are waiting
//             // Remove the thread from the Proc
//             std::cout << "===========> Thread " << Thread->GetThreadID() << " is DONE" << std::endl;
//             // Remove the thread from the Proc
//             // TODO: Save this object to an archive of some sort
//             CompletedThreads.emplace(Thread->GetThreadID());
//             // Remove the thread from the AssignedThreads
//             AssignedThreads.at(i).erase(AssignedThreads.at(i).begin()+j);
//             if( !AssignedThreads.at(i).size() ){
//               Enabled[i] = false;
//             }
//             // Decrement j so we don't skip over the next thread
//             j--;
//             break;
//           }
//         case ThreadState::BLOCKED: {
//             std::cout << "===========> Thread " << Thread->GetThreadID() << " is waiting on Thread " << AssignedThreads.at(i).at(j)->GetWaitingToJoinTID() << std::endl;
//             // Remove the thread from the Proc
//             // std::cout << "===========> Found a thread that's BLOCKED" << std::endl;
//             // TODO: Check for outstanding Loads first
//             // TODO: Add a list of thread ids to RevThread for what it's waiting on and then check that if the entries are in the BlockedThreads list
//             uint32_t BlockedTID = Thread->GetThreadID();
//             if( !CanThreadProceed(BlockTID) ){
//               // Add to BlockedThreads
//               BlockedThreads.emplace(BlockedTID);
//               // Remove from Proc's AssignedThreads
//               AssignedThreads.at(i).erase(AssignedThreads.at(i).begin()+j);

//             }
//
//               // Try to find if this thread is already in the waiting queue
//
//               // uint32_t TID = Thread->GetThreadID();
//               // for( unsigned i=0; i<BlockedThreads.size(); i++ ){
//               //   if( TID == BlockedThreads.at(i) ){
//               //     BlockedThreads.erase(BlockedThreads.begin() + i);
//               //   }
//               //   ThreadQueue.emplace_back(TID);
//               // }
//               break;
//             }
//             else {
//               AssignedThreads.at(i).erase(AssignedThreads.at(i).begin()+j);
//               BlockedThreads.emplace(BlockedTID);
//               break;
//             }
//           }
//           case ThreadState::RUNNING: { // TODO: Handle
//             // std::cout << "===========> Found a thread that's RUNNING" << std::endl;
//             break;
//           }
//           case ThreadState::READY:{ // Thread is ready to be scheduled (ie. it has a TID)
//             // std::cout << "===========> Found a thread that's READY" << std::endl;
//             break;
//           }
//           case ThreadState::START:{ // TODO: Handle
//             break;
//           }
//         default:{
//             // std::cout << "===========> Found a thread that's in an unknown state" << std::endl;
//           break;
//           }
//         }
//       }
//
//       if( Procs[i]->GetUtilization() < 100 && ThreadQueue.size() ){
//         AssignedThreads.at(i).emplace_back(Threads.at(ThreadQueue.front()));
//         ThreadQueue.erase(ThreadQueue.begin());
//       }
//       // Check if we need to create a new thread
//       if( Procs[i]->GetNewThreadInfo().size() ){
//         // std::cout << "Creating a new thread for Proc: " << i << std::endl;
//         // TODO: Figure out if we'll do multiple of these if present (ie. size > 1)
//         std::shared_ptr<RevThread> NewThreadInfo = Procs[i]->GetNewThreadInfo().front();
//         InitThread(NewThreadInfo);

//         // Remove from the queue
//         Procs[i]->GetNewThreadInfo().pop();
//       }
//     }

//     // Proc is disabled, see if we can assign it work
//     else {
//       // std::cout << "PROC IS DISABLED SO I AM ASSIGNING WORK" << std::endl;
//       // See if there is work to assign
//       if( ThreadQueue.size() ){
//         uint32_t ThreadToAssign = ThreadQueue.front();
//         ThreadQueue.erase(ThreadQueue.begin());
//         output.verbose(CALL_INFO, 2, 1, "Assigning Thread %u to Proc %u\n", ThreadToAssign, i);
//         AssignedThreads.at(i).emplace_back(Threads.at(ThreadToAssign));
//         // Enable Proc
//         Enabled[i] = true;
//       }
//     }
//     // Co-Processor Clock Tick
//     if(EnableCoProc && !CoProcs[i]->ClockTick(currentCycle)){
//     output.verbose(CALL_INFO, 5, 0, "Closing Co-Processor %d at Cycle: %" PRIu64 "\n",
//                    i, static_cast<uint64_t>(currentCycle));

//     }
//   } // === End Per Proc tasks
//

//   // TODO: Check if any BlockedThreads have had their counterpart complete execution
//   //       if so, move its TID to the ThreadQueue
//   for( unsigned i=0; i<BlockedThreads.size(); i++ ){
//     std::shared_ptr<RevThread> Thread = Threads.at(BlockedThreads[i]);
//     uint32_t WaitingOnTID = Thread->GetWaitingToJoinTID();
//     if( WaitingOnTID != __INVALID_TID__){
//       if( CompletedThreads.find(WaitingOnTID) != CompletedThreads.end() ){
//         // Stop waiting to join
//         uint32_t ThreadToStart = BlockedThreads[i];
//         Threads.at(ThreadToStart)->SetState(ThreadState::READY);
//         Threads.at(ThreadToStart)->SetWaitingToJoinTID(__INVALID_TID__);
//         BlockedThreads.erase(BlockedThreads.begin()+i);
//         ThreadQueue.emplace_back(ThreadToStart);
//       }
//     }
//   }
//
//   // Clock the PAN network transport module
//   if( EnablePAN ){

//     if( EnableRDMAMBox ){
//       // process the incoming mailbox messages from our local cores
//       if( !PANProcessRDMAMailbox() )
//         output.fatal(CALL_INFO, -1, "Error: failed to process the PAN RDMA mailbox\n");
//     }

//     for( unsigned i=0; i< msgPerCycle; i++ ){
//       // check the mailbox for messages to inject
//       if( !sendPANMessage() )
//         output.fatal(CALL_INFO, -1, "Error: could not send PAN command message\n" );
//     }

//     // process the read queue
//     if( !processPANMemRead() )
//       output.fatal(CALL_INFO, -1, "Error: failed to process the PAN memory read queue\n" );

//     // process the zero address put queue
//     if( !processPANZeroAddr() )
//       output.fatal(CALL_INFO, -1, "Error: failed to process the PAN zero address put queue\n" );
//   }

//   // check to see if we need to inject a fault
//   if( EnableFaults ){
//     if( FaultCntr == 0 ){
//       // inject a fault
//       HandleFaultInjection(currentCycle);

//       // reset the fault counter
//       FaultCntr = fault_width;
//     }else{
//       FaultCntr--;
//     }
//   }

//   // check to see if all the processors are completed
//   for( unsigned i=0; i<Procs.size(); i++ ){
//     if( Enabled[i] ){
//       rtn = false;
//     }
//   }

//   if( ThreadQueue.size() ){
//     std::cout << "Thread Queue: ";
//     for( unsigned i=0; i<ThreadQueue.size(); i++ ){
//       std::cout << ThreadQueue.at(i) << ", ";
//     }
//     std::cout << std::endl;
//     rtn = false;
//   }

//   std::cout << "ThreadQueue.size()" << ThreadQueue.size() << std::endl;
//   std::cout << "BlockedThreads.size()" << BlockedThreads.size() << std::endl;

//   // if no threads in queue, no threads in waiting, and at least 1 thread has completed, we're done
//   if( !ThreadQueue.size() ){
//     rtn = true;
//     if( BlockedThreads.size() ){
//       rtn = false;
//       for( unsigned i=0; i<BlockedThreads.size(); i++ ){
//         std::shared_ptr<RevThread> Thread = Threads.at(BlockedThreads[i]);
//         uint32_t WaitingOnTID = Thread->GetWaitingToJoinTID();
//         if( WaitingOnTID != __INVALID_TID__){
//           if( CompletedThreads.find(WaitingOnTID) != CompletedThreads.end() ){
//             // Stop waiting to join
//             uint32_t ThreadToStart = BlockedThreads[i];
//             Threads.at(ThreadToStart)->SetState(ThreadState::READY);
//             Threads.at(ThreadToStart)->SetWaitingToJoinTID(__INVALID_TID__);
//             BlockedThreads.erase(BlockedThreads.begin()+i);
//             ThreadQueue.emplace_back(ThreadToStart);
//           }
//         } else {
//           Thread->SetState(ThreadState::RUNNING);
//           for( unsigned i=0; i<BlockedThreads.size(); i++ ){
//             ThreadQueue.emplace_back(BlockedThreads[i]);
//             BlockedThreads.erase(BlockedThreads.begin() + i);
//           }
//         }
//       }
//     }
//     if( CompletedThreads.size() ){
//       rtn = true;
//     } else {
//         rtn = false;
//     }
//   }

//
//   // check to see if the network has any outstanding messages: fixme
//   if( !SendMB.empty() || !TrackTags.empty() || !ZeroRqst.empty() || !RevokeHasArrived ){
// #ifdef _REV_DEBUG_
//     if( RevokeHasArrived && !PNic->IsHost() ){
//       if( !SendMB.empty() ){
//         output.verbose(CALL_INFO,5,0,"SendMB not empty\n");
//       }
//       if( !ZeroRqst.empty() ){
//         output.verbose(CALL_INFO,5,0,"ZeroRqst not empty\n");
//       }
//       if( !TrackTags.empty() ){
//         output.verbose(CALL_INFO,5,0,"TrackTags not empty: %d\n", TrackTags.size());
//       }
//     }
// #endif
//     rtn = false;
//   }

//   if( rtn ){
//     // Temporary sanity check
//     for( auto Thread : Threads ){
//       if( Thread.second->GetState() != ThreadState::DONE ){
//         std::cout << "Thread " << Thread.first << "did not finish but the simulation is about to end" << std::endl;
//       }
//     }
//     std::cout << " ThreadQueue Size: " << ThreadQueue.size() << std::endl;
//     std::cout << " BlockedThreads Size: " << BlockedThreads.size() << std::endl;
//     // End execution for all RevProcs
//     for( unsigned i=0; i<numCores; i++ ){
//       Procs[i]->EndExecution();
//     }
//     primaryComponentOKToEndSim();
//     output.verbose(CALL_INFO, 5, 0, "OK to end sim at cycle: %" PRIu64 "\n", static_cast<uint64_t>(currentCycle));
//   }

//   return rtn;
// }
