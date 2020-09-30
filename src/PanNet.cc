//
// _PanNet_cc_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "PanNet.h"

using namespace SST;
using namespace RevCPU;

PanNet::PanNet(ComponentId_t id, Params& params)
  : panNicAPI(id, params) {
  // setup the initial logging functions
  int verbosity = params.find<int>("verbose",0);
  output = new SST::Output("", verbosity, 0, SST::Output::STDOUT);

  registerClock("1Ghz", new Clock::Handler<PanNet>(this,&PanNet::clockTick));

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

  iFace->setNotifyOnReceive(new SST::Interfaces::SimpleNetwork::Handler<PanNet>(this, &PanNet::msgNotify));

  initBroadcastSent = false;

  numDest = 0;

  msgHandler = nullptr;
}

PanNet::~PanNet(){
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
    output->verbose(CALL_INFO, 1, 0,
                    "%s received init message from %s\n",
                    getName().c_str(), ev->getSource().c_str());
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
    if( req != nullptr ){
      panNicEvent *ev = static_cast<panNicEvent*>(req->takePayload());
      delete req;
      (*msgHandler)(ev);
    }
  }
  return true;
}

void PanNet::send(panNicEvent* event, int destination){
  SST::Interfaces::SimpleNetwork::Request *req = new SST::Interfaces::SimpleNetwork::Request();
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

bool PanNet::clockTick(Cycle_t cycle){
  while( !sendQ.empty() ){
    if( iFace->spaceToSend(0,512) && iFace->send(sendQ.front(),0)) {
      sendQ.pop();
    }else{
      break;
    }
  }

  return false;
}

// EOF
