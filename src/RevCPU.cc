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

RevCPU::RevCPU( SST::ComponentId_t id, const SST::Params& params )
  : SST::Component(id), testStage(0), PrivTag(0), address(-1), EnableMemH(false),
    ReadyForRevoke(false), Nic(nullptr), Ctrl(nullptr),
    ClockHandler(nullptr) {

  const int Verbosity = params.find<int>("verbose", 0);

  // Initialize the output handler
  output.init("RevCPU[" + getName() + ":@p:@t]: ", Verbosity, 0, SST::Output::STDOUT);

  // Register a new clock handler
  const std::string cpuClock = params.find<std::string>("clock", "1GHz");
  ClockHandler = new SST::Clock::Handler<RevCPU>(this, &RevCPU::clockTick);
  timeConverter = registerClock(cpuClock, ClockHandler);

  // Inform SST to wait until we authorize it to exit
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Derive the simulation parameters
  // We must always derive the number of cores before initializing the options
  // If the PAN tests are enabled, override the number cores and force them to '0'
  numCores = params.find<unsigned>("numCores", "1");
  output.verbose(CALL_INFO, 1, 0, "Building Rev with %u cores\n", numCores);

  // read the binary executable name
  Exe = params.find<std::string>("program", "a.out");

  // read the program arguments
  Args = params.find<std::string>("args", "");

  // Create the options object
  // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
  Opts = new RevOpts(numCores, Verbosity);
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
    params.find_array<std::string>("table", instTables);
    if( !Opts->InitInstTables( instTables ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the instruction tables\n" );

    std::vector<std::string> memCosts;
    params.find_array<std::string>("memCost", memCosts);
    if( !Opts->InitMemCosts( memCosts ) )
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory latency range\n" );

    std::vector<std::string> prefetchDepths;
    params.find_array<std::string>("prefetchDepth", prefetchDepths);
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

  // Look for the fault injection logic
  EnableFaults = params.find<bool>("enable_faults", 0);
  if( EnableFaults ){
    std::vector<std::string> faults;
    params.find_array<std::string>("faults", faults);
    DecodeFaultCodes(faults);

    std::string width = params.find<std::string>("fault_width", "1");
    DecodeFaultWidth(width);

    fault_width = params.find<int64_t>("fault_range", "65536");
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

  Opts->SetArgs(Loader->GetArgv());

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
      Procs.push_back( new RevProc( i, Opts, Mem, Loader, CoProcs[i], &output ) );
    }
  }else{
    // Create the processor objects
    Procs.reserve(Procs.size() + numCores);
    for( unsigned i=0; i<numCores; i++ ){
      Procs.push_back( new RevProc( i, Opts, Mem, Loader, NULL, &output ) );
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

  // Create the completion array
  Enabled = new bool [numCores];
  for( unsigned i=0; i<numCores; i++ ){
    Enabled[i] = true;
  }

  const unsigned Splash = params.find<bool>("splash", 0);

  if( Splash > 0 )
      output.verbose(CALL_INFO, 1, 0, splash_msg);

  // Done with initialization
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
  if( EnableMemH ){
    Ctrl->setup();
  }
}

void RevCPU::finish(){
}

void RevCPU::init( unsigned int phase ){
  if( EnableNIC )
    Nic->init(phase);
  if( EnableMemH )
    Ctrl->init(phase);
}

void RevCPU::handleMessage(Event *ev){
  nicEvent *event = static_cast<nicEvent*>(ev);
  // -- RevNIC: This is where you can unpack and handle the data payload
  delete event;
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
    if( Enabled[i] ){
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
    if( Enabled[i] )
      rtn = false;
  }

  // check to see if the network has any outstanding messages: fixme
  if( !TrackTags.empty() || !ZeroRqst.empty() ){

#ifdef _REV_DEBUG_
    if( RevokeHasArrived && !PNic->IsHost() ){
      if( !SendMB.empty() ){
        output.verbose(CALL_INFO, 5, 0, "SendMB not empty\n");
      }
      if( !ZeroRqst.empty() ){
        output.verbose(CALL_INFO, 5, 0, "ZeroRqst not empty\n");
      }
      if( !TrackTags.empty() ){
        output.verbose(CALL_INFO, 5, 0, "TrackTags not empty: %zu\n", TrackTags.size());
      }
    }
#endif
    rtn = false;
  }

  if( rtn ){
    primaryComponentOKToEndSim();
    output.verbose(CALL_INFO, 5, 0, "OK to end sim at cycle: %" PRIu64 "\n", currentCycle);
  }

  return rtn;
}

// EOF
