//
// _RevCPU_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevCPU.h"
#include "RevMem.h"
#include "RevThread.h"
#include <cmath>

using namespace SST::RevCPU;
using MemSegment = RevMem::MemSegment;

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
    DisableCoprocClock(false), Nic(nullptr), Ctrl(nullptr), ClockHandler(nullptr) {

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
  numCores = params.find<unsigned>("numCores", "1");
  numHarts = params.find<uint16_t>("numHarts", "1");

  // Make sure someone isn't trying to have more than 65536 harts per core
  if( numHarts > _MAX_HARTS_ ){
    output.fatal(CALL_INFO, -1, "Error: number of harts must be <= %" PRIu32 "\n", _MAX_HARTS_);
  }
  output.verbose(CALL_INFO, 1, 0,
                 "Building Rev with %" PRIu32 " cores and %" PRIu32 " hart(s) on each core \n",
                 numCores, numHarts);

  // read the binary executable name
  Exe = params.find<std::string>("program", "a.out");

  // read the program arguments
  Args = params.find<std::string>("args", "");

  // Create the options object
  // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
  Opts = new RevOpts(numCores, numHarts, Verbosity);
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
      output.fatal(CALL_INFO, -1, "Error: failed to initialize the starting symbols\n" );

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

  AssignedThreads.resize(numCores);
  EnableCoProc = params.find<bool>("enableCoProc", 0);
  if(EnableCoProc){

    // Create the processor objects
    Procs.reserve(Procs.size() + numCores);
    for( unsigned i=0; i<numCores; i++ ){
      Procs.push_back( new RevProc( i, Opts, numHarts, Mem, Loader, AssignedThreads.at(i), this->GetNewTID(), &output ) );
    }
    // Create the co-processor objects
    for( unsigned i=0; i<numCores; i++){
      RevCoProc* CoProc = loadUserSubComponent<RevCoProc>("co_proc", SST::ComponentInfo::SHARE_NONE, Procs[i]);
      if (!CoProc) {
        output.fatal(CALL_INFO, -1, "Error : failed to inintialize the co-processor subcomponent\n");
      }
      CoProcs.push_back(CoProc);
      Procs[i]->SetCoProc(CoProc);
    }
  }else{
    // Create the processor objects
    Procs.reserve(Procs.size() + numCores);
    for( unsigned i=0; i<numCores; i++ ){
      Procs.push_back( new RevProc( i, Opts, numHarts, Mem, Loader, AssignedThreads.at(i), this->GetNewTID(), &output ) );
    }
  }

  #ifndef NO_REV_TRACER
  // Configure tracer and assign to each core
  if (output.getVerboseLevel()>=5) {
    for( unsigned i=0; i<numCores; i++ ){
      // Each core gets its very own tracer
      RevTracer* trc = new RevTracer(getName(), &output);
      std::string diasmType;
      Opts->GetMachineModel(0,diasmType); // TODO first param is core
      if (trc->SetDisassembler(diasmType))
        output.verbose(CALL_INFO, 1, 0, "Warning: tracer could not find disassembler. Using REV default\n");

      trc->SetTraceSymbols(Loader->GetTraceSymbols());

      // tracer user controls - cycle on and off. Ignored unless > 0
      trc->SetStartCycle(params.find<uint64_t>("trcStartCycle",0));
      trc->SetCycleLimit(params.find<uint64_t>("trcLimit",0));
      trc->SetCmdTemplate(params.find<std::string>("trcOp", TRC_OP_DEFAULT).c_str());

      trc->Reset();
      Procs[i]->SetTracer(trc);
    }
  }
  #endif
  // Setup timeConverter
  for( size_t i=0; i<Procs.size(); i++){
    Procs[i]->SetTimeConverter(timeConverter);
  }

  // Initial thread setup
  uint32_t MainThreadID = id+1; // Prevents having MainThreadID == 0 which is reserved for INVALID

  uint64_t StartAddr = 0x00ull;
  std::string StartSymbol;

  bool IsStartSymbolProvided = Opts->GetStartSymbol( id, StartSymbol );
  bool IsStartAddrProvided = Opts->GetStartAddr( id, StartAddr ) && StartAddr != 0x00ull;
  uint64_t ResolvedStartSymbolAddr = (IsStartSymbolProvided) ? Loader->GetSymbolAddr(StartSymbol) : 0x00ull;

  // If no start address has been provided ...
  if (!IsStartAddrProvided) {
    // ... check if symbol was provided ...
    if (!IsStartSymbolProvided) {
        // ... no, try to default to 'main' ...
        StartAddr = Loader->GetSymbolAddr("main");
        if( StartAddr == 0x00ull ){
          // ... no hope left!
          output.fatal(CALL_INFO, -1,
                       "Error: failed to auto discover address for <main> for main thread\n");
        }
    } else {
      // ... if symbol was provided, check whether it is valid or not ...
      if (!ResolvedStartSymbolAddr) {
        // ... not valid, error out
        output.fatal(CALL_INFO, -1,
                  "Error: failed to resolve address for symbol <%s>\n", StartSymbol.c_str());
      }
      // ... valid use the resolved symbol
      StartAddr = ResolvedStartSymbolAddr;
    }
  } else { // A start address was provided ...
    // ... check if a symbol was provided and is compatible with the start address ...
    if ((IsStartSymbolProvided) && (ResolvedStartSymbolAddr != StartAddr)) {
      // ... they are different, don't know the user intent so error out now
      output.fatal(CALL_INFO, -1,
                  "Error: start address and start symbol differ startAddr=0x%" PRIx64 " StartSymbol=%s ResolvedStartSymbolAddr=0x%" PRIx64 "\n",
                   StartAddr, StartSymbol.c_str(), ResolvedStartSymbolAddr);
    } // ... else no symbol provided, continue on with StartAddr as the target
  }

  output.verbose(CALL_INFO, 11, 0, "Start address is 0x%" PRIx64 "\n", StartAddr);

  std::shared_ptr<RevThread> MainThread = std::make_shared<RevThread>(MainThreadID,                    // ThreadID
                                                                      _INVALID_TID_,                 // Parent ThreadID
                                                                      Mem->GetStackTop(),              // Stack Pointer
                                                                      StartAddr,                       // PC
                                                                      Mem->GetThreadMemSegs().front(), // ThreadMemSeg pointer
                                                                      Procs[0]->GetRevFeature());      // RevFeature


  InitThread(MainThread);

  output.verbose(CALL_INFO, 11, 0, "Main thread initialized %s\n", MainThread->to_string().c_str());
  SetupArgs(MainThreadID, Procs[0]->GetRevFeature());

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

  // determine whether we need to enable/disable manual coproc clocking
  DisableCoprocClock = params.find<bool>("independentCoprocClock", 0);

  // Create the completion array
  Enabled = new bool [numCores];
  for( unsigned i=0; i<numCores; i++ ){
    Enabled[i] = false;
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
    Core = RevRand(0, numCores-1);
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
    Core = RevRand(0, numCores-1);
  }

  Procs[Core]->HandleRegFault(fault_width);
}

void RevCPU::HandleALUFault(SST::Cycle_t currentCycle){
  output.verbose(CALL_INFO, 4, 0, "FAULT: ALU fault injected at cycle: %" PRIu64 "\n",
                 currentCycle);

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ){
    Core = RevRand(0, numCores-1);
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

void RevCPU::UpdateCoreStatistics(unsigned coreNum){
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
  for( size_t i=0; i<Procs.size(); i++ ){
    // Check if we have more work to assign and places to put it
    UpdateThreadAssignments(i);
    if( Enabled[i] ){
      if( !Procs[i]->ClockTick(currentCycle) ){
        if(EnableCoProc && !CoProcs.empty()){
          CoProcs[i]->Teardown();
        }
        UpdateCoreStatistics(i);
        Enabled[i] = false;
        output.verbose(CALL_INFO, 5, 0, "Closing Processor %zu at Cycle: %" PRIu64 "\n",
                       i, currentCycle);
      }
      if(EnableCoProc &&
         !CoProcs[i]->ClockTick(currentCycle) &&
         !DisableCoprocClock){
        output.verbose(CALL_INFO, 5, 0, "Closing Co-Processor %zu at Cycle: %" PRIu64 "\n",
                       i, currentCycle);

      }
    }

    // See if any of the threads on this proc changes state
    CheckForThreadStateChanges(i);

    if( Procs[i]->GetHartUtilization() == 0 ){
      Enabled[i] = false;
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
  for( size_t i=0; i<Procs.size(); i++ ){
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
      Threads.at(*(BlockedThreads.begin()))->SetWaitingToJoinTID(_INVALID_TID_);
      ThreadQueue.emplace_back(*BlockedThreads.begin());
      BlockedThreads.erase(BlockedThreads.begin());
    }
    rtn = false;
  }

  // check to see if the network has any outstanding messages: fixme
  if( !TrackTags.empty() || !ZeroRqst.empty() ){
    rtn = false;
  }

  if( rtn && CompletedThreads.size() ){
    for( unsigned i=0; i<numCores; i++ ){
      Procs[i]->PrintStatSummary();
    }
    primaryComponentOKToEndSim();
    output.verbose(CALL_INFO, 5, 0, "OK to end sim at cycle: %" PRIu64 "\n", static_cast<uint64_t>(currentCycle));
  } else {
    rtn = false;
  }

  return rtn;
}


// Initializes a RevThread object.
// - Moves it to the 'Threads' map
// - Adds it's ThreadID to the ThreadQueue to be scheduled
void RevCPU::InitThread(std::shared_ptr<RevThread>& ThreadToInit){

  auto gp = Loader->GetSymbolAddr("__global_pointer$");
  ThreadToInit->GetRegFile()->SetX(RevReg::gp, gp);
  ThreadToInit->GetRegFile()->SetX(RevReg::s0, gp);

  uint32_t TID = ThreadToInit->GetThreadID();
  // Check if this ThreadID has already been assigned... if so... something has gone horribly wrong
  // print out all threads
  auto it = Threads.find(TID);
  if( it != Threads.end() && it->second->GetState() != ThreadState::START ){
    output.fatal(CALL_INFO, 99, "Error: ThreadID %" PRIu32 " has already been assigned... this is a bug.\n", TID);
  }
  output.verbose(CALL_INFO, 4, 0, "Initializing Thread %" PRIu32 "\n", TID);
  output.verbose(CALL_INFO, 11, 0, "Thread Information: %s", ThreadToInit->to_string().c_str());
  ThreadToInit->SetState(ThreadState::READY);
  Threads.emplace(ThreadToInit->GetThreadID(), ThreadToInit);
  ThreadQueue.emplace_back(TID);
}

void RevCPU::AssignThread(uint32_t ThreadID, uint32_t ProcID){
  output.verbose(CALL_INFO, 4, 0, "Assigning Thread %" PRIu32 " to Processor %" PRIu32 "\n", ThreadID, ProcID);
  auto Thread = Threads.at(ThreadID);

  // Point the regfile of this thread's LSQ to the Proc's LSQ
  Thread->GetRegFile()->SetLSQueue( Procs[ProcID]->GetLSQueue() );

  // Point thread's regfile to this proc's MarkLoadComplete
  Thread->GetRegFile()->SetMarkLoadComplete([proc = Procs[ProcID]](const MemReq& req){ proc->MarkLoadComplete(req); });

  // Put the thread in the Proc's assigned threads list
  AssignedThreads.at(ProcID).emplace(ThreadID, Thread);

  Procs[ProcID]->AssignThread(ThreadID);
  return;

}

// Checks if a thread with a given Thread ID can proceed (used for pthread_join).
// it does this by seeing if a given thread's WaitingOnTID has completed
bool RevCPU::ThreadCanProceed(uint32_t TID){
  bool rtn = false;

  // Get the thread's waiting to join TID
  uint32_t WaitingOnTID = (Threads.at(TID))->GetWaitingToJoinTID();

  // If the thread is waiting on another thread, check if that thread has completed
  if( WaitingOnTID != _INVALID_TID_ ){
    // Check if WaitingOnTID has completed... if so, return = true, else return false
    output.verbose(CALL_INFO, 4, 0, "Thread %" PRIu32 " is waiting on Thread %u\n", TID, WaitingOnTID);

    // Check if the WaitingOnTID has completed, if not, thread cannot proceed
    rtn = ( CompletedThreads.find(WaitingOnTID) != CompletedThreads.end() ) ? true : false;
  }
  // If the thread is not waiting on another thread, it can proceed
  else {
    // Thread is waiting on INVALID_TID (ie. No thread)
    // so it can proceed
    rtn = true;
  }

  return rtn;
}

// Check if any BlockedThreads have had their counterpart complete execution
// if so, move its TID to the ThreadQueue
void RevCPU::CheckBlockedThreads(){
  // Iterate over all block threads
  for( auto ThreadID : BlockedThreads ){
    // Check if the thread can proceed (ie. its WaitingOnTID has completed)
    if( ThreadCanProceed(ThreadID) ){
      // Mark thread as ready (no longer blocked)
      Threads.at(ThreadID)->SetState(ThreadState::READY);
      // Remove the waiting to join TID
      Threads.at(ThreadID)->SetWaitingToJoinTID(_INVALID_TID_);
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

// ----------------------------------
// We need to initialize the x10 register to include the value of ARGC
// This is >= 1 (the executable name is always included)
// We also need to initialize the ARGV pointer to the value
// of the ARGV base pointer in memory which is currently set to the
// program header region.  When we come out of reset, this is StackTop+60 bytes
// ----------------------------------
void RevCPU::SetupArgs(uint32_t ThreadIDToSetup, RevFeature* feature){
  auto Argv = Opts->GetArgv();
  Threads.at(ThreadIDToSetup)->GetRegFile()->SetX(RevReg::a0, Argv.size());
  Threads.at(ThreadIDToSetup)->GetRegFile()->SetX(RevReg::a1, Mem->GetStackTop() + 60);
}

// Checks core 'i' to see if it has any available harts to assign work to
// if it does and there is work to assign (ie. ThreadQueue is not empty)
// assign it and enable the processor if not already enabled.
void RevCPU::UpdateThreadAssignments(uint32_t ProcID){
  // print the thread queue
  // Get utilization info
  double Util = Procs[ProcID]->GetHartUtilization();
  //if( Util > 0.0 ){
    output.verbose(CALL_INFO, 11, 0, "Core %" PRIu32 " utilization: %.2f%%\n", ProcID, Util);
  //}
  // Check if we have room to schedule another thread
  if( Util < 100  ){
    output.verbose(CALL_INFO, 10, 0, "Core %" PRIu32 " utilization: %.2f%%\n", ProcID, Util);
    // We can schedule another thread
    // Check if we have any threads to schedule
    if( ThreadQueue.size() ){
      // Add to this proc's thread list
      Threads.at(ThreadQueue.front())->SetState(ThreadState::RUNNING);
      AssignThread(ThreadQueue.front(), ProcID);
      // AssignedThreads.at(ProcID).emplace_back(Threads.at(ThreadQueue.front()));
      // output.verbose(CALL_INFO, 6, 1, "Assigning Thread %u to Core %u\n", ThreadQueue.front(), ProcID);
      // Remove from thread queue
      ThreadQueue.erase(ThreadQueue.begin());
      // If this Proc was previously disabled, enable it
      // if( !Enabled[i] ){ Enabled[i] = true; }
      Enabled[ProcID] = true;
    }
  } // Utilization is 100%, so change nothing
  return;
}

// Checks for state changes in the threads of a given processor index 'i'
// and handle appropriately
void RevCPU::CheckForThreadStateChanges(uint32_t ProcID){
  // Handle any thread state changes for this core
  // NOTE: At this point we handle EVERY thread that changed state every cycle
  while( !Procs[ProcID]->GetThreadsThatChangedState().empty() ){
    auto& Thread = Procs[ProcID]->GetThreadsThatChangedState().front();
    // Handle the thread that changed state based on the new state
    switch ( Thread->GetState() ) {
    case ThreadState::DONE:
      // This thread has completed execution
      // We need to:
      // 1. Remove it from the AssignedThreads map (The Hart will automatically be updated)
      // 2. Move its ThreadID to the CompletedThreads list
      output.verbose(CALL_INFO, 8, 0, "Thread %" PRIu32 " on Core %" PRIu32 " is DONE\n", Thread->GetThreadID(), ProcID);
      AssignedThreads.at(ProcID).erase(Thread->GetThreadID());
      CompletedThreads.emplace(Thread->GetThreadID());
      if( AssignedThreads.at(ProcID).empty() ){
        Enabled[ProcID] = false;
      }
      break;
    case ThreadState::BLOCKED:
      // This thread is blocked (currently only caused by a rev_pthread_join)
      // We need to:
      // 1. Check if the thread it is waiting on has already completed
      // 2. If it has... Thread can resume execution
      // 3. If not, thread remains blocked
      //    3a. Move its ThreadID to the BlockedThreads list
      //    3b. Remove it from the AssignedThreads lis
      output.verbose(CALL_INFO, 8, 0, "Thread %" PRIu32 "on Core %" PRIu32 " is BLOCKED\n", Thread->GetThreadID(), ProcID);
      // -- 1.
      if( ThreadCanProceed(Thread->GetThreadID()) ){
        // -- 2.
        output.verbose(CALL_INFO, 8, 0, "Thread %" PRIu32 " on Core %" PRIu32 " was waiting on thread %u which has already completed so it can proceed\n",
                        Thread->GetThreadID(), ProcID, Thread->GetWaitingToJoinTID());
        // Continue executing thread on same Core
        Thread->SetState(ThreadState::RUNNING);
      }
      else { // -- 3.
        output.verbose(CALL_INFO, 8, 0, "Thread %" PRIu32 " on Core %" PRIu32 " was waiting on thread %u which has not yet completed so it remains blocked\n",
                        Thread->GetThreadID(), ProcID, Thread->GetWaitingToJoinTID());
        Thread->SetState(ThreadState::BLOCKED);
        // -- 3a.
        BlockedThreads.emplace(Thread->GetThreadID());

        // -- 3b.
        AssignedThreads.at(ProcID).erase(Thread->GetThreadID());

        if( AssignedThreads.at(ProcID).empty() ){
          Enabled[ProcID] = false;
        }

      }
      break;
    case ThreadState::START: // Should never happen
      output.verbose(CALL_INFO, 99, 1, "A new thread with ID = %" PRIu32 " was found on Core %" PRIu32, Thread->GetThreadID(), ProcID);

      // Mark it ready for execution
      Thread->SetState(ThreadState::READY);

      // Add it to the Thread map
      Threads.emplace(Thread->GetThreadID(), Thread);

      // Add it to the thread queue to be scheduled
      ThreadQueue.emplace_back(Thread->GetThreadID());
      break;

    case ThreadState::RUNNING:
      output.verbose(CALL_INFO, 11, 0, "Thread %" PRIu32 " on Core %" PRIu32 " is RUNNING\n", Thread->GetThreadID(), ProcID);
      break;

    case ThreadState::READY:
      // If this happens we are not setting state when assigning thread somewhere
      output.fatal(CALL_INFO, 99, "Error: Thread %" PRIu32 " on Core %" PRIu32 " is assigned but is in READY state... This is a bug\n",
                    Thread->GetThreadID(), ProcID);
      break;
    default: // Should DEFINITELY never happen
      output.fatal(CALL_INFO, 99, "Error: Thread %" PRIu32 " on Core %" PRIu32 " is in an unknown state... This is a bug\n",
                    Thread->GetThreadID(), ProcID);
      break;
    }
    // Pop the thread that changed state
    Procs[ProcID]->GetThreadsThatChangedState().pop();
  }
  return;
}

// EOF
