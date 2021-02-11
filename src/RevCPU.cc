//
// _RevCPU_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevCPU.h"

const char *splash_msg = "\
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

const char *pan_splash_msg = "\
\n\
       __|__\n\
--@--@--(_)--@--@--\n\
\n\
    PAN PAN PAN!\n\
";

RevCPU::RevCPU( SST::ComponentId_t id, SST::Params& params )
  : SST::Component(id), testStage(0), address(-1),
    EnableNIC(false), EnablePAN(false), EnablePANStats(false),
    Nic(nullptr), PNic(nullptr), PExec(nullptr) {

  const int Verbosity = params.find<int>("verbose", 0);

  // Initialize the output handler
  output.init("RevCPU[" + getName() + ":@p:@t]: ", Verbosity, 0, SST::Output::STDOUT);

  // Determine whether we're running the test harness
  EnablePANTest = params.find<bool>("enable_test", 0);

  // Register a new clock handler
  std::string cpuClock = params.find<std::string>("clock", "1GHz");
  if( EnablePANTest ){
    timeConverter  = registerClock(cpuClock, new SST::Clock::Handler<RevCPU>(this,&RevCPU::clockTickPANTest));
    testIters = params.find<unsigned>("testIters", 255);
  }else{
    timeConverter  = registerClock(cpuClock, new SST::Clock::Handler<RevCPU>(this,&RevCPU::clockTick));
  }

  // Inform SST to wait unti we authorize it to exit
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Derive the simulation parameters
  // We must always derive the number of cores before initializing the options
  // If the PAN tests are enabled, override the number cores and force them to '0'
  numCores = params.find<unsigned>("numCores", "1");
  if( EnablePANTest )
    numCores = 1; // force the PAN test to use a single core

  // read the binary executable name
  Exe = params.find<std::string>("program", "a.out");

  // read the program arguments
  Args = params.find<std::string>("args", "");

  // Create the options object
  Opts = new RevOpts(numCores,Verbosity);
  if( !Opts )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RevOpts object\n" );

  // Initialize the remaining options
  std::vector<std::string> startAddrs;
  params.find_array<std::string>("startAddr", startAddrs);
  if( !Opts->InitStartAddrs( startAddrs ) )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the starting addresses\n" );

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
      PExec = new PanExec();
      for( unsigned i=0; i<Procs.size(); i++ ){
        Procs[i]->SetExecCtx(PExec);
      }
    }

    // record the number of injected messages per cycle
    msgPerCycle = params.find<unsigned>("msgPerCycle", 1);
    RDMAPerCycle = params.find<unsigned>("RDMAPerCycle", 1);
    EnableRDMAMBox = params.find<bool>("enableRDMAMbox",1);

    if( EnablePANStats )
      registerStatistics();
  }

  // See if we should load the test harness as opposed to a binary payload
  if( EnablePANTest && (!EnablePAN) ){
    output.fatal(CALL_INFO, -1, "Error: enabling PAN tests requires a pan_nic");
  }

  // Create the memory object
  unsigned long memSize = params.find<unsigned long>("memSize", 1073741824);
  Mem = new RevMem( memSize, Opts,  &output );
  if( !Mem )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory object\n" );

  // Load the binary into memory
  Loader = new RevLoader( Exe, Args, Mem, &output );
  if( !Loader ){
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RISC-V loader\n" );
  }

  // Create the processor objects
  for( unsigned i=0; i<numCores; i++ ){
    Procs.push_back( new RevProc( i, Opts, Mem, Loader, &output ) );
  }

  // Create the completion array
  Enabled = new bool [numCores];
  for( unsigned i=0; i<numCores; i++ ){
    Enabled[i] = true;
  }

  unsigned Splash = params.find<bool>("splash",0);

  if( Splash > 0 ){
    if( EnablePANTest )
      output.verbose(CALL_INFO,1,0,pan_splash_msg);
    else
      output.verbose(CALL_INFO,1,0,splash_msg);
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
  for( unsigned i=0; i<Procs.size(); i++ ){
    delete Procs[i];
  }

  if( PExec )
    delete PExec;

  // delete the memory object
  delete Mem;

  // delete the loader object
  if( Loader )
    delete Loader;

  // delete the options object
  delete Opts;
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
  }
}

void RevCPU::finish(){
}

void RevCPU::init( unsigned int phase ){
  if( EnableNIC )
    Nic->init(phase);
  if( EnablePAN )
    PNic->init(phase);
}

void RevCPU::handleMessage(Event *ev){
  nicEvent *event = static_cast<nicEvent*>(ev);
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

void RevCPU::PANHandleSuccess(panNicEvent *event){
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

  output.verbose(CALL_INFO, 8, 0,
                 "FAILED RESPONSE: Found matching tag and source identifier for incoming message: tag=%d; src=%d\n",
                 event->getTag(),
                 event->getSrc());

  // remove the entry
  TrackTags.erase(it);
}

void RevCPU::PANBuildFailedToken(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Building failed return packet\n");
  panNicEvent *FEvent = new panNicEvent();
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
  panNicEvent *SEvent = new panNicEvent();
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
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
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
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // write it to memory
  if( !Mem->WriteMem(event->getAddr(), Size, (void *)(Data)) ){
    delete[] Data;
    PANBuildFailedToken(event);
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleAsyncGet(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
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
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // write it to memory
  if( !Mem->WriteMem(event->getAddr(), Size, (void *)(Data)) ){
    delete[] Data;
    PANBuildFailedToken(event);
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleSyncStreamGet(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
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
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // write it to memory
  if( !Mem->WriteMem(event->getAddr(), Size, (void *)(Data)) ){
    delete[] Data;
    PANBuildFailedToken(event);
    return ;
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleAsyncStreamGet(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
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
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  // retrieve the data
  uint32_t Size = event->getSize();
  uint64_t *Data = new uint64_t [event->getNumBlocks(Size)];
  event->getData(Data);

  // write it to memory
  if( !Mem->WriteMem(event->getAddr(), Size, (void *)(Data)) ){
    delete[] Data;
    PANBuildFailedToken(event);
    return ;
  }

  // build response
  delete[] Data;
  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Size);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleExec(panNicEvent *event){
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

  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Idx);
  SCmd->setSrc(address);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleStatus(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }
  unsigned Idx = (unsigned)(event->getSize());
  PanExec::PanStatus Status = PExec->StatusEntry(Idx);
  if( Status == PanExec::QNull ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent();
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

  unsigned Idx = (unsigned)(event->getSize());
  if( !PExec->RemoveEntry(Idx) ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent();
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

  PNic->RevokeToken();
  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleHalt(panNicEvent *event){
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

  panNicEvent *SCmd = new panNicEvent();

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

  panNicEvent *SCmd = new panNicEvent();
  PANBuildBasicSuccess(event,SCmd);
  SCmd->setAddr(PC);
  SCmd->setSrc(address);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleSetFuture(panNicEvent *event){
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
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  bool IsFuture = Mem->StatusFuture(event->getAddr());
  panNicEvent *SCmd = new panNicEvent();
  PANBuildBasicSuccess(event,SCmd);
  if( IsFuture )
    SCmd->setSize(0x01);
  else
    SCmd->setSize(0x00);
  SCmd->setSrc(address);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleBOTW(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
    return ;
  }

  uint8_t VarArgs = event->getVarArgs();
  uint64_t Entry  = (uint64_t)(event->getOffset()) + Loader->GetSymbolAddr("_start");
  unsigned Idx    = 0;

  // marshall the varargs to the device
  // TODO

  if( !PExec->AddEntry(Entry,&Idx) ){
    PANBuildFailedToken(event);
    return ;
  }

  panNicEvent *SCmd = new panNicEvent();
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
  case panNicEvent::SyncPut:
  case panNicEvent::AsyncGet:
  case panNicEvent::AsyncPut:
  case panNicEvent::SyncStreamGet:
  case panNicEvent::SyncStreamPut:
  case panNicEvent::AsyncStreamGet:
  case panNicEvent::AsyncStreamPut:
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
                 "Error: host devices cannot handle %s commands\n",
                 event->getOpcodeStr().c_str() );
    break;
  }
}

void RevCPU::handleNetPANMessage(panNicEvent *event){
  output.verbose(CALL_INFO, 5, 0, "Handling PAN Message opcode=%d\n", (int)(event->getOpcode()));
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
  case panNicEvent::Failed:
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

  output.verbose(CALL_INFO, 4, 0, "Sending PAN message from %d to %d; Opc=%s; Tag=%d\n",
                 address, SendMB.front().second,
                 SendMB.front().first->getOpcodeStr().c_str(),
                 SendMB.front().first->getTag());
  PNic->send(SendMB.front().first,SendMB.front().second);
  if( PNic->IsHost() ){
    // save the message to track the response
    TrackTags.push_back(std::make_pair(SendMB.front().first->getTag(),
                                       SendMB.front().second));
  }

  if( EnablePANStats )
    registerSendCmd(SendMB.front().first);

  // pop the message off the queue
  SendMB.pop();

  return true;
}

bool RevCPU::processPANMemRead(){
  // walk the entire vector and decrement all the counters
  // if we have a counter decrement to '0', then process the read request
  // send responses as necessary
  std::vector<std::tuple<uint8_t,uint32_t,unsigned,int,uint64_t>>::iterator it;

  for( it=ReadQueue.begin(); it != ReadQueue.end(); ++it ){
    std::get<2>(*it) = std::get<2>(*it)-1;
    if( std::get<2>(*it) == 0 ){
      // process this read request
      uint8_t tmp_tag = std::get<0>(*it);
      uint32_t tmp_size = std::get<1>(*it);
      int tmp_src = std::get<3>(*it);
      uint64_t tmp_addr = std::get<4>(*it);

      // -- setup the response
      panNicEvent *SCmd = new panNicEvent();
      uint64_t *Data = new uint64_t [SCmd->getNumBlocks(tmp_size)];
      if( !Mem->ReadMem( tmp_addr,
                         (size_t)(tmp_size),
                         (void *)(Data))){
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
    }
  }

  // process all the deletions separately
  for( it=ReadQueue.begin(); it != ReadQueue.end(); ++it ){
    if( std::get<2>(*it) == 0 ){
      ReadQueue.erase(it);
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
                 "Coverting RDMA Mailbox Entry to Event: Payload = %llu; Opcode=%d\n",
                 Payload, Opcode);

  // Stage 2: use the opcode the read and encode the remainder of the data
  switch(Opcode){
  case panNicEvent::SyncGet:
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !event->buildSyncGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SyncGet; Tag=%d\n",Tag);
    break;
  case panNicEvent::SyncPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !Mem->ReadMem(Addr+16,Size,Data) ){
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
    if( !event->buildAsyncGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA AsyncGet; Tag=%d\n",Tag);
    break;
  case panNicEvent::AsyncPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !Mem->ReadMem(Addr+16,Size,Data) ){
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
    if( !event->buildSyncStreamGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA SyncStreamGet; Tag=%d\n",Tag);
    break;
  case panNicEvent::SyncStreamPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !Mem->ReadMem(Addr+8,Size,Data) ){
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
    if( !event->buildAsyncStreamGet(Token,Tag,CmdAddr,Size) )
      output.fatal(CALL_INFO, -1,
                  "Error: could not build RDMA AsyncStreamGet; Tag=%d\n",Tag);
    break;
  case panNicEvent::AsyncStreamPut:
    Data = new uint64_t [event->getNumBlocks(Size)];
    CmdAddr = Mem->ReadU64(Addr+8);
    if( !Mem->ReadMem(Addr+8,Size,Data) ){
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
  uint64_t Addr = _PAN_RDMA_MAILBOX_;
  uint64_t Payload[3];
  uint64_t CmdBuf;
  uint64_t Buf = 0x00ull;
  panNicEvent *TEvent = nullptr;

  while( !done ){

    //
    // retriver the 'iter' 24-byte mailbox payload
    // each payload is configured as follows:
    //      ADDR + 16          ADDR + 8             ADDR
    // [  EVENT POINTER  ][      DEST       ][      VALID      ]
    //
    // If `Valid` != 0; The entry is valid, consume it
    // Else, continue
    //

    // Stage 1: Read the memory
    Mem->ReadMem(Addr,24,&Payload[0]);

    // Stage 2: Interrogate the payload
    if( Payload[0] == _PAN_ENTRY_VALID_ ){
      // found a valid payload, process it
      output.verbose(CALL_INFO, 8, 0, "Processing RDMA Mailbox Command\n");

      // Stage 2.a: Convert the buffer into an event
      panNicEvent *FEvent = new panNicEvent();

      if( !PANConvertRDMAtoEvent(Payload[2],FEvent) )
        output.fatal(CALL_INFO, -1,
                     "Error: could not convert RDMA command to event from address=0x%llx\n",
                     (long long unsigned int)(Payload[1]));

      // Stage 2.b: Insert the event into the send queue
      SendMB.push(std::make_pair(FEvent,(int)(Payload[1])));

      // Stage 2.c: mark the payload as being completed
      Payload[0] = _PAN_ENTRY_INJECTED_;
      Mem->WriteMem(Addr,8,&Payload[0]);

      // Stage 2.d: increment the message counter
      sent++;
    }// else invalid or injected

    // Stage 3: Update the counters & the Address
    iter++;
    Addr += 24;
    Payload[0] = 0x00ull;
    Payload[1] = 0x00ull;
    Payload[2] = 0x00ull;

    if( iter == _PAN_RDMA_MAX_ENTRIES_ ){
      done = true;
    }else if( sent == RDMAPerCycle ){
      done = true;
    }else if( SendMB.size() == 255 ){
      done = true;
    }
  }

  return true;
}

uint8_t RevCPU::createTag(){
  if( PrivTag == 0b11111111 ){
    return 0b00000000;
  }else{
    PrivTag +=1;
    return PrivTag;
  }
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
    TEvent = new panNicEvent();
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
      TEvent = new panNicEvent();  // new event to send
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
    TEvent = new panNicEvent();
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
    Mem->WriteU64((uint64_t)(_PAN_COMPLETION_ADDR_),0xdeadbeef);
    break;
  case 10:
    // revoke reservation
    output.verbose(CALL_INFO, 4, 0, "Executing REVOKE test\n");
    TEvent = new panNicEvent();
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
  output.verbose(CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", static_cast<uint64_t>(currentCycle));

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
      (testStage < _MAX_PAN_TEST_) )
    rtn = false;

  // if its time to return, end the sim
  if( rtn )
    primaryComponentOKToEndSim();

  return rtn;
}

bool RevCPU::clockTick( SST::Cycle_t currentCycle ){
  bool rtn = true;

  output.verbose(CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", static_cast<uint64_t>(currentCycle));

  // Execute each enabled core
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] ){
      if( !Procs[i]->ClockTick(currentCycle) )
        Enabled[i] = false;
    }
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
  }

  // check to see if all the processors are completed
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] )
      rtn = false;
  }

  // check to see if the network has any outstanding messages
  if( !SendMB.empty() || !TrackTags.empty() )
    rtn = false;

  if( rtn )
    primaryComponentOKToEndSim();

  return rtn;
}

// EOF
