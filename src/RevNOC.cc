//
// _RevNOC_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "RevCPU.h"
#include "RevNOC.h"
#include <ostream>

using namespace SST;
using namespace RevCPU;

RevNOC::RevNOC(ComponentId_t id, Params& params)
  : RevNocAPI(id, params) {
  // setup the initial logging functions
  int verbosity = params.find<int>("verbose", 0);
  output = new SST::Output("", verbosity, 0, SST::Output::STDOUT);

  const std::string nocClock = params.find<std::string>("clock", "1GHz");
  registerClock(nocClock, new Clock::Handler<RevNOC>(this, &RevNOC::ClockTick));

  // load the SimpleNetwork interfaces
  iFace = loadUserSubComponent<SST::Interfaces::SimpleNetwork>("iface", ComponentInfo::SHARE_NONE, 1);
  if( !iFace ){
    // load the anonymous noc
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

  iFace->setNotifyOnReceive(
    new SST::Interfaces::SimpleNetwork::Handler<RevNOC>(this, &RevNOC::msgRecvNotify));

  initBroadcastSent = false;

  numDest = 0;

  msgHandler = nullptr;
}

RevNOC::~RevNOC(){
  delete output;
}

void RevNOC::setMsgHandler(Event::HandlerBase* handler){
  msgHandler = handler;
}

void RevNOC::init(unsigned int phase){
  // convert the std::cout's to output->verbose's
  iFace->init(phase);

  if( iFace->isNetworkInitialized() ){
    output->verbose(CALL_INFO, 1, 0, "%s network is initialized\n", getName().c_str());
    if( !initBroadcastSent) {
      initBroadcastSent = true;
      std::vector<uint64_t> sendP;
      sendP.push_back(LogicalID);
      sendP.push_back((uint64_t)(iFace->getEndpointID()));
      RevNOCPkt *Pkt = new RevNOCPkt(RevNOCPacketType::INIT_BCAST, LogicalID, sendP);

      SST::Interfaces::SimpleNetwork::Request * req = new SST::Interfaces::SimpleNetwork::Request();
      req->dest = SST::Interfaces::SimpleNetwork::INIT_BROADCAST_ADDR;
      req->src = iFace->getEndpointID();
      req->givePayload(Pkt);
      iFace->sendUntimedData(req);

      // add myself
      hostMap[LogicalID] = iFace->getEndpointID();
    }
  }

  while( SST::Interfaces::SimpleNetwork::Request * req =
         iFace->recvUntimedData() ) {
    RevNOCPkt *Pkt = static_cast<RevNOCPkt*>(req->takePayload());
    numDest++;
    std::vector<uint64_t> RecvPkt = Pkt->getData();
    hostMap[RecvPkt[0]] = req->src;
    output->verbose(CALL_INFO, 1, 0,
                    "%s received init message from logical ID=%" PRIu64 "\n",
                    getName().c_str(), RecvPkt[0]);
  }

  if( phase == 4 ){
    output->verbose(CALL_INFO, 9, 0,
                  "------------------------------------------------------\n");
    output->verbose(CALL_INFO, 9, 0, "REVNOC NETWORK MAPPING\n");
    output->verbose(CALL_INFO, 9, 0,
                  "------------------------------------------------------\n");
    for( auto const & [key, val] : hostMap ){
      output->verbose(CALL_INFO, 9, 0,
                      "Endpoint Logical ID=%" PRIu64 " == Physical ID=%" PRIu64 "\n",
                      key, (uint64_t)(val));
    }
    output->verbose(CALL_INFO, 9, 0,
                  "------------------------------------------------------\n");
  }
}

void RevNOC::setup(){
  if( msgHandler == nullptr ){
    output->fatal(CALL_INFO, -1,
                  "%s, Error: RevNOC implements a callback-based notification and parent has not registered a callback function\n",
                  getName().c_str());
  }
}

void RevNOC::AckThisPkt(const uint64_t MsgIDToAck, const uint64_t DestLogicalID){
  std::vector<uint64_t> AckData;
  AckData.push_back(MsgIDToAck);
  RevNOCPkt *AckPkt = new RevNOCPkt(RevNOCPacketType::ACK, LogicalID, AckData);
  send(AckPkt, DestLogicalID);
}

enum class AckFmt : uint64_t {
  MSGID = 0,
  SRCID = 1,
  TYPE = 2,
};

void RevNOC::ProcessAck(RevNOCPkt *pkt){
  auto Data = pkt->getData();
  const uint64_t AckedMsgID = Data.at((size_t)AckFmt::MSGID);

  // Remove from list of outstanding acks
  if( OutstandingAcks.find(AckedMsgID) != OutstandingAcks.end() ){
    OutstandingAcks.erase(AckedMsgID);
  } else {
    output->fatal(CALL_INFO, -1,
                "%s, Error: RevNOC: received ack for unknown message ID=%" PRIu64 "\n",
                getName().c_str(), AckedMsgID);

  }
}

bool RevNOC::msgRecvNotify(int vn){
  SST::Interfaces::SimpleNetwork::Request* req = iFace->recv(0);
  if( req == nullptr ){
    return false;
  }

  RevNOCPkt *Pkt = static_cast<RevNOCPkt*>(req->takePayload());
  if( Pkt->getType() == RevNOCPacketType::DATA ){
    // output the data packet
    std::vector<uint64_t> PktData = Pkt->getData();
    // convert to string
    std::string str = VecU64ToString(PktData);
    AckThisPkt(Pkt->getMsgID(), Pkt->getSrc());
  } else if( Pkt->getType() == RevNOCPacketType::ACK ){
    ProcessAck(Pkt);
  } else if( Pkt->getType() == RevNOCPacketType::INIT_BCAST ){
    std::vector<uint64_t> RecvPkt = Pkt->getData();
    hostMap[RecvPkt[0]] = req->src;
    output->verbose(CALL_INFO, 1, 0,
                    "%s received init message from logical ID=%" PRIu64 "\n",
                    getName().c_str(), RecvPkt[0]);
  } else {
    output->fatal(CALL_INFO, -1,
                  "%s, Error: RevNOC: unknown packet type\n",
                  getName().c_str());
  }

  (*msgHandler)(Pkt);

  return true;
}

void RevNOC::send(RevNOCPkt* event, uint64_t destination){

  // check to make sure the destination is valid
  bool found = false;
  auto destID = 0;
  for( auto i : hostMap ){
    if( i.first == destination ){
      destID = i.second;
      found = true;
    }
  }

  if( !found ){
    output->fatal(CALL_INFO, -1,
                  "%s, Error: RevNOC: unknown logical destination %" PRIu64 "\n",
                  getName().c_str(), destination);
  }

  output->verbose(CALL_INFO, 1, 0,
                  "%s sending message to logical destination %" PRIu64 "\n",
                  getName().c_str(), destination);

  // NOTE: Only ACKing DATA packets for now
  if( event->getType() == RevNOCPacketType::DATA){
    OutstandingAcks.insert(event->getMsgID());
  }

  SST::Interfaces::SimpleNetwork::Request *req =
    new SST::Interfaces::SimpleNetwork::Request();
  req->dest = destID;
  req->src = iFace->getEndpointID();
  req->givePayload(event);
  sendQ.push(req);
}

void RevNOC::sendString(const std::string& str, RevNOCPacketType Type, uint64_t dest){
  std::vector<uint64_t> PktData = StringToVecU64(str);
  RevNOCPkt *Pkt = new RevNOCPkt(Type, LogicalID, PktData);
  send(Pkt, dest);
}

std::vector<uint64_t> RevNOC::StringToVecU64(const std::string& str) {
  std::vector<uint64_t> result;
  for (size_t i = 0; i < str.size(); i += 8) {
    uint64_t packed = 0;
    for (size_t j = 0; j < 8 && i + j < str.size(); ++j) {
      packed |= static_cast<uint64_t>(str[i + j]) << (j * 8);
    }
    result.emplace_back(packed);
  }
  return result;
}

std::string RevNOC::VecU64ToString(const std::vector<uint64_t>& vec) {
  std::string result;
  for (uint64_t packed : vec) {
    for (size_t j = 0; j < 8; ++j) {
      char c = static_cast<char>((packed >> (j * 8)) & 0xFF);
      if (c != 0) {  // Assuming null-termination for strings
        result += c;
      }
    }
  }
  return result;
}

uint64_t RevNOC::getNumDestinations(){
  return numDest;
}

SST::Interfaces::SimpleNetwork::nid_t RevNOC::getAddress(){
  return iFace->getEndpointID();
}

bool RevNOC::ClockTick(Cycle_t cycle){
  while( !sendQ.empty() ){
    RevNOCPkt *Pkt = static_cast<RevNOCPkt *>(sendQ.front()->inspectPayload());
    std::vector<uint64_t> PktData  = Pkt->getData();
    if( iFace->spaceToSend(0, PktData.size()*64) && iFace->send(sendQ.front(), 0)) {
      sendQ.pop();
    }else{
      break;
    }
  }

  return false;
}

// EOF
