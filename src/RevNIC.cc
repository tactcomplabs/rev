//
// _RevNIC_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevNIC.h"

using namespace SST;
using namespace RevCPU;

RevNIC::RevNIC(ComponentId_t id, Params& params)
  : nicAPI(id, params) {
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

  iFace->setNotifyOnReceive(new SST::Interfaces::SimpleNetwork::Handler<RevNIC>(this, &RevNIC::msgNotify));

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
      nicEvent *ev = new nicEvent(getName());

      SST::Interfaces::SimpleNetwork::Request * req = new SST::Interfaces::SimpleNetwork::Request();
      req->dest = SST::Interfaces::SimpleNetwork::INIT_BROADCAST_ADDR;
      req->src = iFace->getEndpointID();
      req->givePayload(ev);
      iFace->sendInitData(req);
    }
  }

  while( SST::Interfaces::SimpleNetwork::Request * req = iFace->recvInitData() ) {
    nicEvent *ev = static_cast<nicEvent*>(req->takePayload());
    numDest++;
    output->verbose(CALL_INFO, 1, 0,
                    "%s received init message from %s\n",
                    getName().c_str(), ev->getSource().c_str());
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
  if( req != nullptr ){
    if( req != nullptr ){
      nicEvent *ev = static_cast<nicEvent*>(req->takePayload());
      delete req;
      (*msgHandler)(ev);
    }
  }
  return true;
}

void RevNIC::send(nicEvent* event, int destination){
  SST::Interfaces::SimpleNetwork::Request *req = new SST::Interfaces::SimpleNetwork::Request();
  req->dest = destination;
  req->src = iFace->getEndpointID();
  req->givePayload(event);
  sendQ.push(req);
}

int RevNIC::getNumDestinations(){
  return numDest;
}

SST::Interfaces::SimpleNetwork::nid_t RevNIC::getAddress(){
  return iFace->getEndpointID();
}

bool RevNIC::clockTick(Cycle_t cycle){
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
