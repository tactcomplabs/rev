//
// _RevCPU_cc_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
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

RevCPU::RevCPU( SST::ComponentId_t id, SST::Params& params )
  : SST::Component(id), EnableNIC(false) {

  const int Verbosity = params.find<int>("verbose", 0);

  // Initialize the output handler
  output.init("RevCPU[" + getName() + ":@p:@t]: ", Verbosity, 0, SST::Output::STDOUT);

  // Register a new clock handler
  std::string cpuClock = params.find<std::string>("clock", "1GHz");
  timeConverter  = registerClock(cpuClock, new SST::Clock::Handler<RevCPU>(this,&RevCPU::clockTick));

  // Inform SST to wait unti we authorize it to exit
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Derive the simulation parameters
  // We must always derive the number of cores before initializing the options
  numCores = params.find<unsigned>("numCores", "0");

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

  if( EnablePAN ){
    // Look up the network component

    PNic = loadUserSubComponent<panNicAPI>("pan_nic");

    // check to see if the nic was loaded.  if not, DO NOT load an anonymous endpoint
    if(!Nic)
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
  }

  // Create the memory object
  unsigned long memSize = params.find<unsigned long>("memSize", 1073741824);
  Mem = new RevMem( memSize, Opts,  &output );
  if( !Mem )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory object\n" );

  // Load the binary into memory
  Loader = new RevLoader( Exe, Args, Mem, &output );
  if( !Loader )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RISC-V loader\n" );

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
    output.verbose(CALL_INFO,1,0,splash_msg);
  }

  // Done with initialization
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
  delete Loader;

  // delete the options object
  delete Opts;
}

void RevCPU::setup(){
  if( EnableNIC )
    Nic->setup();
  if( EnablePAN )
    PNic->setup();
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

void RevCPU::PANBuildFailedToken(panNicEvent *event){
  panNicEvent *FEvent = new panNicEvent();
  if( !FEvent->buildFailed(event->getToken(),event->getTag()) ){
    output.fatal(CALL_INFO, -1,
                 "Error: failed to construct token failure command for tag=%d\n",
                 event->getTag());
  }
  SendMB.push(std::make_pair(FEvent,event->getSrc()));
}

void RevCPU::PANBuildRawSuccess(panNicEvent *event){
  panNicEvent *SEvent = new panNicEvent();
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
}

void RevCPU::PANHandleSyncPut(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleAsyncGet(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleAsyncPut(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleSyncStreamGet(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleSyncStreamPut(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleAsyncStreamGet(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleAsyncStreamPut(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::PANHandleExec(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }

  unsigned Idx = 0;
  if( !PExec->AddEntry(event->getAddr(),&Idx) )
    PANBuildFailedToken(event);

  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Idx);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleStatus(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
  unsigned Idx = (unsigned)(event->getSize());
  PanExec::PanStatus Status = PExec->StatusEntry(Idx);
  if( Status == PanExec::QNull )
    PANBuildFailedToken(event);

  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize((uint32_t)(Status));
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleCancel(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }

  unsigned Idx = (unsigned)(event->getSize());
  if( !PExec->RemoveEntry(Idx) )
    PANBuildFailedToken(event);

  panNicEvent *SCmd = new panNicEvent();
  SCmd->setSize(Idx);
  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleReserve(panNicEvent *event){
  if( PNic->IsReserved() ){
    // send failed response
    PANBuildFailedToken(event);
  }

  if( !PNic->SetToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }

  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleRevoke(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }

  PNic->RevokeToken();
  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleHalt(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
  }
  Procs[HART]->Halt();
  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleResume(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
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
  }
  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
  }

  unsigned Idx = (unsigned)(event->getAddr());
  uint64_t Value = 0x00ull;

  if( !Procs[HART]->DebugReadReg(Idx,&Value) )
    PANBuildFailedToken(event);

  panNicEvent *SCmd = new panNicEvent();

  uint32_t Width = 0;
  if( Procs[HART]->DebugIsRV32() )
    Width = 4;
  else
    Width = 8;

  SCmd->setData(&Value,Width);
  SCmd->setSize(Width);

  PANBuildBasicSuccess(event,SCmd);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleWriteReg(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
  }

  unsigned Idx = (unsigned)(event->getAddr());
  uint64_t Value = 0x00ull;
  event->getData(&Value);

  if( !Procs[HART]->DebugWriteReg(Idx,Value) )
    PANBuildFailedToken(event);

  PANBuildRawSuccess(event);
}

void RevCPU::PANHandleSingleStep(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
  unsigned HART = event->getSize();
  if( HART > (numCores-1) ){
    PANBuildFailedToken(event);
  }
  uint64_t PC = Procs[HART]->GetPC();
  if( !Procs[HART]->SingleStepHart() ){
    PANBuildFailedToken(event);
    return ;
  }
  panNicEvent *SCmd = new panNicEvent();
  PANBuildBasicSuccess(event,SCmd);
  SCmd->setAddr(PC);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleSetFuture(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
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
  }

  bool IsFuture = Mem->StatusFuture(event->getAddr());
  panNicEvent *SCmd = new panNicEvent();
  PANBuildBasicSuccess(event,SCmd);
  if( IsFuture )
    SCmd->setSize(0x01);
  else
    SCmd->setSize(0x00);
  SendMB.push(std::make_pair(SCmd,event->getSrc()));
}

void RevCPU::PANHandleBOTW(panNicEvent *event){
  if( !PNic->CheckToken(event->getToken()) ){
    // send failed response
    PANBuildFailedToken(event);
  }
}

void RevCPU::handleHostPANMessage(panNicEvent *event){
  switch( event->getOpcode() ){
  case panNicEvent::Success:
    break;
  case panNicEvent::Failed:
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
  switch( event->getOpcode() ){
  case panNicEvent::SyncGet:
    PANHandleSyncGet(event);
    break;
  case panNicEvent::SyncPut:
    PANHandleSyncPut(event);
    break;
  case panNicEvent::AsyncGet:
    PANHandleAsyncGet(event);
    break;
  case panNicEvent::AsyncPut:
    PANHandleAsyncPut(event);
    break;
  case panNicEvent::SyncStreamGet:
    PANHandleSyncStreamGet(event);
    break;
  case panNicEvent::SyncStreamPut:
    PANHandleSyncStreamPut(event);
    break;
  case panNicEvent::AsyncStreamGet:
    PANHandleAsyncStreamGet(event);
    break;
  case panNicEvent::AsyncStreamPut:
    PANHandleAsyncStreamPut(event);
    break;
  case panNicEvent::Exec:
    PANHandleExec(event);
    break;
  case panNicEvent::Status:
    PANHandleStatus(event);
    break;
  case panNicEvent::Cancel:
    PANHandleCancel(event);
    break;
  case panNicEvent::Reserve:
    PANHandleReserve(event);
    break;
  case panNicEvent::Revoke:
    PANHandleRevoke(event);
    break;
  case panNicEvent::Halt:
    PANHandleHalt(event);
    break;
  case panNicEvent::Resume:
    PANHandleResume(event);
    break;
  case panNicEvent::ReadReg:
    PANHandleReadReg(event);
    break;
  case panNicEvent::WriteReg:
    PANHandleWriteReg(event);
    break;
  case panNicEvent::SingleStep:
    PANHandleSingleStep(event);
    break;
  case panNicEvent::SetFuture:
    PANHandleSetFuture(event);
    break;
  case panNicEvent::RevokeFuture:
    PANHandleRevokeFuture(event);
    break;
  case panNicEvent::StatusFuture:
    PANHandleStatusFuture(event);
    break;
  case panNicEvent::BOTW:
    PANHandleBOTW(event);
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

bool RevCPU::sendPANMessage(){
  // check the mailbox for the next message
  if( SendMB.empty() )
    return true;

  PNic->send(SendMB.front().first,SendMB.front().second);

  SendMB.pop();

  return true;
}

bool RevCPU::processPANMemRead(){
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

  // TODO: check to see if the network has any outstanding messages

  // TODO: clear the memory write queue

  if( rtn )
    primaryComponentOKToEndSim();

  return rtn;
}

// EOF
