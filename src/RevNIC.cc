//
// _RevNIC_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevNIC.h"

using namespace SST;
using namespace RevCPU;

RevNIC::RevNIC(ComponentId_t id, Params& params)
  : RevNicAPI(id, params) {
  // setup the initial logging functions
  int verbosity = params.find<int>("verbose", 0);
  output = new SST::Output("", verbosity, 0, SST::Output::STDOUT);

  const std::string nicClock = params.find<std::string>("clock", "1GHz");
  registerClock(nicClock, new Clock::Handler<RevNIC>(this, &RevNIC::clockTick));

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

  iFace->setNotifyOnReceive(
    new SST::Interfaces::SimpleNetwork::Handler<RevNIC>(this, &RevNIC::msgNotify));

  initBroadcastSent = false;

  numDest = 0;

  msgHandler = nullptr;
}

RevNIC::~RevNIC(){
  delete output;
}

void RevNIC::setMsgHandler(Event::HandlerBase* handler){
  msgHandler = handler;
}

void RevNIC::init(unsigned int phase){
  iFace->init(phase);

  if( iFace->isNetworkInitialized() ){
    if( !initBroadcastSent) {
      initBroadcastSent = true;
      std::vector<uint64_t> sendP;
      sendP.push_back(ID);
      sendP.push_back((uint64_t)(iFace->getEndpointID()));
      RevPkt *ev = new RevPkt(PacketType::INIT_BCAST, ID, sendP);

      SST::Interfaces::SimpleNetwork::Request * req = new SST::Interfaces::SimpleNetwork::Request();
      req->dest = SST::Interfaces::SimpleNetwork::INIT_BROADCAST_ADDR;
      req->src = iFace->getEndpointID();
      req->givePayload(ev);
      iFace->sendUntimedData(req);

      // add myself
      hostMap[ID] = iFace->getEndpointID();
    }
  }

  while( SST::Interfaces::SimpleNetwork::Request * req =
         iFace->recvUntimedData() ) {
    RevPkt *ev = static_cast<RevPkt*>(req->takePayload());
    numDest++;
    std::vector<uint64_t> recvP = ev->getData();
    hostMap[recvP[0]] = req->src;
    output->verbose(CALL_INFO, 1, 0,
                    "%s received init message from logical ID=%" PRIu64 "\n",
                    getName().c_str(), recvP[0]);
  }

  if( phase == 4 ){
    output->verbose(CALL_INFO, 9, 0,
                  "------------------------------------------------------\n");
    output->verbose(CALL_INFO, 9, 0, "REVNIC NETWORK MAPPING\n");
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

void RevNIC::setup(){
  if( msgHandler == nullptr ){
    output->fatal(CALL_INFO, -1,
                  "%s, Error: RevNIC implements a callback-based notification and parent has not registerd a callback function\n",
                  getName().c_str());
  }
}

bool RevNIC::msgNotify(int vn){
  SST::Interfaces::SimpleNetwork::Request* req = iFace->recv(0);
  if( req == nullptr ){
    return false;
  }

  RevPkt *ev = static_cast<RevPkt*>(req->takePayload());
  (*msgHandler)(ev);

  return true;
}

void RevNIC::send(RevPkt* event, uint64_t destination){

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
                  "%s, Error: RevNIC: unknown logical destination %" PRIu64 "\n",
                  getName().c_str(), destination);
  }

  SST::Interfaces::SimpleNetwork::Request *req =
    new SST::Interfaces::SimpleNetwork::Request();
  req->dest = destID;
  req->src = iFace->getEndpointID();
  req->givePayload(event);
  sendQ.push(req);
}

uint64_t RevNIC::getNumDestinations(){
  return numDest;
}

SST::Interfaces::SimpleNetwork::nid_t RevNIC::getAddress(){
  return iFace->getEndpointID();
}

bool RevNIC::clockTick(Cycle_t cycle){
  while( !sendQ.empty() ){
    RevPkt *ev = static_cast<RevPkt *>(sendQ.front()->inspectPayload());
    auto P = ev->getData();
    if( iFace->spaceToSend(0, P.size()*64) && iFace->send(sendQ.front(), 0)) {
      sendQ.pop();
    }else{
      break;
    }
  }

  return false;
}


// EOF
