//
// _PanNet_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/PanNet.h"

using namespace SST;
using namespace RevCPU;


// -----------------------------------------------------------------------------
// panNicEvent Class
// -----------------------------------------------------------------------------

panNicEvent::PanPacket panNicEvent::getType(){
  uint8_t Type = Opcode & 0b11;
  return (panNicEvent::PanPacket)(Type);
}

std::string panNicEvent::getOpcodeStr(){
  switch(Opcode){
  case panNicEvent::Success:
    return "Success";
    break;
  case panNicEvent::Failed:
    return "Failed";
    break;
  case panNicEvent::SyncGet:
    return "SyncGet";
    break;
  case panNicEvent::SyncPut:
    return "SyncPut";
    break;
  case panNicEvent::AsyncGet:
    return "AsyncGet";
    break;
  case panNicEvent::AsyncPut:
    return "AsyncPut";
    break;
  case panNicEvent::SyncStreamGet:
    return "SyncStreamGet";
    break;
  case panNicEvent::SyncStreamPut:
    return "SyncStreamPut";
    break;
  case panNicEvent::AsyncStreamGet:
    return "AsyncStreamGet";
    break;
  case panNicEvent::AsyncStreamPut:
    return "AsyncStreamPut";
    break;
  case panNicEvent::Exec:
    return "Exec";
    break;
  case panNicEvent::Status:
    return "Status";
    break;
  case panNicEvent::Cancel:
    return "Cancel";
    break;
  case panNicEvent::Reserve:
    return "Reserve";
    break;
  case panNicEvent::Revoke:
    return "Revoke";
    break;
  case panNicEvent::Halt:
    return "Halt";
    break;
  case panNicEvent::Resume:
    return "Resume";
    break;
  case panNicEvent::ReadReg:
    return "ReadReg";
    break;
  case panNicEvent::WriteReg:
    return "WriteReg";
    break;
  case panNicEvent::SingleStep:
    return "SingleStep";
    break;
  case panNicEvent::SetFuture:
    return "SetFuture";
    break;
  case panNicEvent::RevokeFuture:
    return "RevokeFuture";
    break;
  case panNicEvent::StatusFuture:
    return "StatusFuture";
    break;
  case panNicEvent::BOTW:
    return "BOTW";
    break;
  default:
    return "UNKNOWN";
    break;
  }
}

bool panNicEvent::setData(uint64_t *In, uint32_t Sz){
  unsigned blocks = 0;

  if( Sz == 0 )
    return true;

  blocks = getNumBlocks(Sz);
  for( unsigned i=0; i<blocks; i++ ){
    Data.push_back(In[i]);
  }

  return true;
}

void panNicEvent::getData(uint64_t *Out){
  unsigned blocks = 0;

  if( Size == 0 )
    return ;

  blocks = getNumBlocks(Size);

  for( unsigned i=0; i<blocks; i++ ){
    Out[i] = Data[i];
  }
}

unsigned panNicEvent::getNumBlocks(uint32_t Sz){
  unsigned blocks = 0;

  if( Sz == 0 ){
    blocks = 0;
  }else if( Sz < 8 ){
    blocks = 1;
  }else{
    blocks = Sz/8;
    if( (Sz%8)>0 )
      blocks += 1;
  }

  return blocks;
}

bool panNicEvent::setTag(uint8_t T){
  Tag = T;
  return true;
}

bool panNicEvent::setVarArgs(uint8_t VA){
  VarArgs = (VA&0b1111);
  return true;
}

bool panNicEvent::setSize(uint32_t Sz){
  switch(getType()){
  case PanBase:
    Size = (Sz &0b1111111111111111);
    break;
  case PanStream:
    Size = (Sz &0b11111111111111111111);
    break;
  case PanRsvd:
  case PanBOTW:
  default:
    Size = 0;
    return false;
    break;
  }
  return true;
}

bool panNicEvent::setToken(uint32_t T){
  Token = T;
  return true;
}

bool panNicEvent::setOffset(uint32_t O){
  Offset = (O&0b111111111111111111);
  return true;
}

bool panNicEvent::setAddr(uint64_t A){
  Addr = A;
  return true;
}

bool panNicEvent::buildSyncGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size){
  Opcode = SyncGet;

  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;

  return true;
}

bool panNicEvent::buildSyncPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data){
  Opcode = SyncPut;
  if( Data == nullptr ){
    return false;
  }
  if( !setToken(Token) ){
    return false;
  }
  if( !setTag(Tag) ){
    return false;
  }
  if( !setSize(Size) ){
    return false;
  }
  if( !setAddr(Addr) ){
    return false;
  }
  if( !setData(Data, Size) ){
    return false;
  }

  return true;
}

bool panNicEvent::buildAsyncGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size){
  Opcode = AsyncGet;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;

  return true;
}

bool panNicEvent::buildAsyncPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data){
  Opcode = AsyncPut;
  if( Data == nullptr )
    return false;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;
  if( !setData(Data, Size) )
    return false;

  return true;
}

bool panNicEvent::buildSyncStreamGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size){
  Opcode = SyncStreamGet;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;

  return true;
}

bool panNicEvent::buildSyncStreamPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data){
  Opcode = SyncStreamPut;
  if( Data == nullptr )
    return false;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;
  if( !setData(Data, Size) )
    return false;

  return true;
}

bool panNicEvent::buildAsyncStreamGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size){
  Opcode = AsyncStreamGet;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;

  return true;
}

bool panNicEvent::buildAsyncStreamPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data){
  Opcode = AsyncStreamPut;
  if( Data == nullptr )
    return false;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;
  if( !setData(Data, Size) )
    return false;

  return true;
}

bool panNicEvent::buildExec(uint32_t Token, uint8_t Tag, uint64_t Addr){
  Opcode = Exec;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(Size) )
    return false;
  if( !setAddr(Addr) )
    return false;

  return true;
}

bool panNicEvent::buildStatus(uint32_t Token, uint8_t Tag, uint16_t Entry){
  Opcode = Status;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize((uint32_t)(Entry)) )
    return false;
  return true;
}

bool panNicEvent::buildCancel(uint32_t Token, uint8_t Tag, uint16_t Entry){
  Opcode = Cancel;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize((uint32_t)(Entry)) )
    return false;
  return true;
}

bool panNicEvent::buildReserve(uint32_t Token, uint8_t Tag){
  Opcode = Reserve;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  return true;
}

bool panNicEvent::buildRevoke(uint32_t Token, uint8_t Tag){
  Opcode = Revoke;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  return true;
}

bool panNicEvent::buildHalt(uint32_t Token, uint8_t Tag, unsigned Hart){
  Opcode = Halt;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize((uint32_t)(Hart)) )
    return false;
  return true;
}

bool panNicEvent::buildResume(uint32_t Token, uint8_t Tag){
  Opcode = Resume;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  return true;
}

bool panNicEvent::buildReadReg(uint32_t Token, uint8_t Tag, unsigned Hart, uint64_t Reg){
  Opcode = ReadReg;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize((uint32_t)(Hart)) )
    return false;
  if( !setAddr(Reg) )
    return false;
  return true;
}

bool panNicEvent::buildWriteReg(uint32_t Token, uint8_t Tag, unsigned Hart, uint64_t Reg, uint64_t *Data){
  Opcode = WriteReg;
  if( Data == nullptr )
    return false;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize((uint32_t)(Hart)) )
    return false;
  if( !setAddr(Reg) )
    return false;
  if( !setData(Data, 8) )      /// right now we only write 8 bytes
    return false;
  return true;
}

bool panNicEvent::buildSingleStep(uint32_t Token, uint8_t Tag, unsigned Hart){
  Opcode = SingleStep;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize((uint32_t)(Hart)) )
    return false;
  return true;
}

bool panNicEvent::buildSetFuture(uint32_t Token, uint8_t Tag, uint64_t Addr){
  Opcode = SetFuture;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setAddr(Addr) )
    return false;
  return true;
}

bool panNicEvent::buildRevokeFuture(uint32_t Token, uint8_t Tag, uint64_t Addr){
  Opcode = RevokeFuture;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setAddr(Addr) )
    return false;
  return true;
}

bool panNicEvent::buildStatusFuture(uint32_t Token, uint8_t Tag, uint64_t Addr){
  Opcode = StatusFuture;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setAddr(Addr) )
    return false;
  return true;
}

bool panNicEvent::buildBOTW(uint32_t Token, uint8_t Tag, uint8_t VarArgs, uint64_t *Args, uint32_t Offset){
  Opcode = BOTW;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setOffset(Offset) )
    return false;
  if( !setData(Args, (uint32_t)(VarArgs)) )
    return false;
  return true;
}

bool panNicEvent::buildSuccess(uint32_t Token, uint8_t Tag){
  Opcode = Success;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(0x00) )
    return false;
  return true;
}

bool panNicEvent::buildFailed(uint32_t Token, uint8_t Tag){
  Opcode = Failed;
  if( !setToken(Token) )
    return false;
  if( !setTag(Tag) )
    return false;
  if( !setSize(0x00) )
    return false;
  return true;
}

// -----------------------------------------------------------------------------
// PanNet Class
// -----------------------------------------------------------------------------

PanNet::PanNet(ComponentId_t id, Params& params)
  : panNicAPI(id, params) {
  // setup the initial logging functions
  int verbosity = params.find<int>("verbose", 0);
  output = new SST::Output("", verbosity, 0, SST::Output::STDOUT);

  registerClock("1Ghz", new Clock::Handler<PanNet>(this, &PanNet::clockTick));

  // load the SimpleNetwork interfaces
  iFace = loadUserSubComponent<SST::Interfaces::SimpleNetwork>("iface", ComponentInfo::SHARE_NONE, 1);
  if( !iFace ){
    // load the anonymous nic
    Params netparams;
    netparams.insert("port_name", params.find<std::string>("port", "network"));
    netparams.insert("in_buf_size", "256B");
    netparams.insert("out_buf_size", "256B");
    netparams.insert("link_bw", "40GiB/s");
    iFace = loadAnonymousSubComponent<SST::Interfaces::SimpleNetwork>("merlin.linkcontrol",
                                                                      "iface",
                                                                      0,
                                                                      ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
                                                                      netparams,
                                                                      1);
  }

  // determine if this is a host device
  isHost = params.find<bool>("host_device", 0);
  this->SetHost(isHost);

  iFace->setNotifyOnReceive(new SST::Interfaces::SimpleNetwork::Handler<PanNet>(this, &PanNet::msgNotify));

  initBroadcastSent = false;

  numDest = 0;

  msgHandler = nullptr;
}

PanNet::~PanNet(){
  delete output;
}

void PanNet::setMsgHandler(Event::HandlerBase* handler){
  msgHandler = handler;
}

void PanNet::init(unsigned int phase){
  iFace->init(phase);

  if( iFace->isNetworkInitialized() ){
    if( !initBroadcastSent) {
      initBroadcastSent = true;
      panNicEvent *ev = new panNicEvent(getName());
      ev->setTag((uint8_t)(this->IsHost()));    // send a boolean on whether we are a host device

      SST::Interfaces::SimpleNetwork::Request * req = new SST::Interfaces::SimpleNetwork::Request();
      req->dest = SST::Interfaces::SimpleNetwork::INIT_BROADCAST_ADDR;
      req->src = iFace->getEndpointID();
      req->givePayload(ev);
      iFace->sendInitData(req);
    }
  }

  while( SST::Interfaces::SimpleNetwork::Request * req = iFace->recvInitData() ) {
    panNicEvent *ev = static_cast<panNicEvent*>(req->takePayload());
    numDest++;
    SST::Interfaces::SimpleNetwork::nid_t srcID = req->src;
    hostMap[srcID] = ev->getTag();
    output->verbose(CALL_INFO, 1, 0,
                    "%s received PAN init message from %s\n",
                    getName().c_str(), ev->getSource().c_str());
    if( (bool)(ev->getTag()) ){
      output->verbose(CALL_INFO, 1, 0,
                      "%s identified %s as a HOST device\n",
                      getName().c_str(), ev->getSource().c_str());
    }else{
      output->verbose(CALL_INFO, 1, 0,
                      "%s identified %s as a PAN device\n",
                      getName().c_str(), ev->getSource().c_str());
    }
  }

}

void PanNet::setup(){
  if( msgHandler == nullptr ){
    output->fatal(CALL_INFO, -1,
                  "%s, Error: PanNet implements a callback-based notification and parent has not registerd a callback function\n",
                  getName().c_str());
  }
}

bool PanNet::msgNotify(int vn){
  SST::Interfaces::SimpleNetwork::Request* req = iFace->recv(0);
  if( req != nullptr ){
    panNicEvent *ev = static_cast<panNicEvent*>(req->takePayload());
    if( !ev ){
      output->fatal(CALL_INFO, -1, "%s, Error: panNicEvent on PanNet is null\n",
                    getName().c_str());
    }
    output->verbose(CALL_INFO, 9, 0,
                    "%s received message opcode of %d from %s\n",
                    getName().c_str(), ev->getOpcode(), ev->getSource().c_str());
    (*msgHandler)(ev);
    delete req;
  }
  return true;
}

void PanNet::send(panNicEvent* event, int destination){
  SST::Interfaces::SimpleNetwork::Request *req = new SST::Interfaces::SimpleNetwork::Request();
  output->verbose(CALL_INFO, 9, 0,
                  "%s sending message opcode of %d from %s\n",
                  getName().c_str(), event->getOpcode(), event->getSource().c_str());
  req->dest = destination;
  req->src = iFace->getEndpointID();
  req->givePayload(event);
  sendQ.push(req);
}

int PanNet::getNumDestinations(){
  return numDest;
}

SST::Interfaces::SimpleNetwork::nid_t PanNet::getAddress(){
  return iFace->getEndpointID();
}

int64_t PanNet::getHostFromIdx(unsigned Idx){
  if( Idx > (hostMap.size()-1) ){
    return -1;
  }

  unsigned i = 0;
  for( std::map<SST::Interfaces::SimpleNetwork::nid_t, bool>::iterator it = hostMap.begin();
       it != hostMap.end();
       ++it ){
    if( i == Idx ){
      return it->first;
    }
    i++;
  }

  return -1;
}

bool PanNet::clockTick(Cycle_t cycle){
  while( !sendQ.empty() ){
    if( iFace->spaceToSend(0, 512) && iFace->send(sendQ.front(), 0)) {
      sendQ.pop();
    }else{
      break;
    }
  }

  return false;
}

// EOF
