//
// _RevNIC_cc_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevNIC.h"

RevNIC::RevNIC( SST::ComponentId_t id, SST::Params& params )
  : SST::Component(id),
    PacketsSent(0), PacketsRecv(0), StalledCycles(0), ExpRecvCount(0),
    Done(false), Init(false), InitCount(0), InitBCastCount(0),
    output(Simulation::getSimulation()->getSimulationOutput()) {

  // init all the standard parameters
  NetId = params.find<int>("id",-1);
  if( NetId == -1 )
    output.fatal(CALL_INFO, -1, "RevNIC: failed to find 'id' parameter\n");

  NumPeers = params.find<int>("num_peers",-1);
  if( NumPeers == -1 )
    output.fatal(CALL_INFO, -1, "RevNIC: failed to find 'num_peers' parameter\n");

  SendUntimedBCast = params.find<bool>("send_untimed_data","false");

  UnitAlgebra TmpMsgSize = params.find<std::string>("message_size","64b");
  if( TmpMsgSize.hasUnits("B") )
    TmpMsgSize *= UnitAlgebra("8b/B");
  MsgSize = TmpMsgSize.getRoundedValue();

  // Link Control
  LinkControl = loadUserSubComponent<SST::Interfaces::SimpleNetwork>("networkIF",
                                                                     ComponentInfo::SHARE_NONE,
                                                                     1);
  if( !LinkControl )
    output.fatal(CALL_INFO, -1, "RevNIC: no link_control object loaded into RevNIC\n");

  LastTarget = NetId;
  NextSeq = new int[NumPeers];
  for( int i=0; i<NumPeers; i++ )
    NextSeq[i] = 0;

  // register the clock
  registerClock( "1Ghz", new Clock::Handler<RevNIC>(this,&RevNIC::ClockHandler), false );

  // register the component
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();
}

RevNIC::~RevNIC(){
  delete LinkControl;
  delete [] NextSeq;
}

void RevNIC::finish(){
  if( InitCount != NumPeers )
    output.output("RevNIC NIC %d didn't receive all complete p2p messages.  Received %d\n",
                  NetId,
                  InitCount);

  if( SendUntimedBCast ){
    if( InitBCastCount != (NumPeers-1) )
      output.output("RevNIC NIC %d didn't receive all complete bcast messages. Received %d\n",
                    NetId,
                    InitBCastCount);
  }

  LinkControl->finish();
  output.output("RevNIC NIC %d had %d stalled cycles.\n",
                NetId,
                StalledCycles);
}

void RevNIC::setup(){
  LinkControl->setup();

  if( LinkControl->getEndpointID() != NetId )
    output.fatal(CALL_INFO, -1,
                 "RevNIC: NIC ids don't match: param = %" PRIi64 ", LinkControl = %" PRIi64 "\n",
                 (int64_t)(NetId), (int64_t)(LinkControl->getEndpointID()));

  if( InitCount != NumPeers )
    output.output("RevNIC: NIC %d didn't receive all p2p messages. Only received %d\n",
                  NetId, InitCount);

  if( !SendUntimedBCast )
    return ;

  if( InitBCastCount != (NumPeers-1) )
    output.output("RevNIC: NIC %d didn't receive all init bcast messages. Only received %d\n",
                  NetId,
                  InitBCastCount);

  LastTarget = id;
}

void RevNIC::complete(unsigned int phase){
  LinkControl->complete(phase);

  if( phase == 0 ){
    InitCount = 0;
    InitBCastCount = 0;
    InitState = 0;
  }

  InitComplete(phase);
}

void RevNIC::init(unsigned int phase){
  LinkControl->init(phase);
  InitComplete(phase);
}

void RevNIC::InitComplete(unsigned int phase){
  SimpleNetwork::Request *req = nullptr;
  switch( InitState ){
  case 0:
    // wait until the network comes up
    if( !LinkControl->isNetworkInitialized() )
      break;
    NetId = LinkControl->getEndpointID();
    LastTarget = NetId;

    if( NetId == 0 ){
      if( SendUntimedBCast ){
        SimpleNetwork::Request* req =
          new SimpleNetwork::Request(SimpleNetwork::INIT_BROADCAST_ADDR, NetId,
                                     0, true, true );
        LinkControl->sendUntimedData(req);
        InitState = 2;
      }else{
        for( int i = 0; i<NumPeers; ++i ){
          SimpleNetwork::Request* req =
            new SimpleNetwork::Request(i, NetId, 0, true, true );
          LinkControl->sendUntimedData(req);
        }
        InitState = 4;
      }
    }else{
      if( SendUntimedBCast ){
        InitState = 1;
      }else{
        InitState = 3;
      }
    }
    break;
  case 1:
    while( (req = LinkControl->recvUntimedData() ) != NULL ){
      delete req;
      InitBCastCount++;
    }

    if( InitBCastCount >= NetId ){
      SimpleNetwork::Request* req =
        new SimpleNetwork::Request(SimpleNetwork::INIT_BROADCAST_ADDR, NetId,
                                   0, true, true);
      LinkControl->sendUntimedData(req);
      InitState = 2;
    }
    break;
  case 2:
    while ( (req = LinkControl->recvUntimedData() ) != NULL ) {
      if( req->dest == SimpleNetwork::INIT_BROADCAST_ADDR ) {
        InitBCastCount++;
      }else{
        if( req->dest != NetId )
          output.output("%d: received event with dest %ld and src %ld\n",
                        NetId,
                        req->dest,
                        req->src);
        InitCount++;
      }
      delete req;
    }

    if( InitBCastCount == (NumPeers-1) ){
      if( NetId == 0 ){
        for( int i=0; i<NumPeers; ++i ){
          req = new SimpleNetwork::Request(i, NetId, 0, true, true);
          LinkControl->sendUntimedData(req);
        }
        InitState = 4;
      }else{
        InitState = 3;
      }
    }
    break;
  case 3:
    while( (req = LinkControl->recvUntimedData()) != NULL ){
      InitCount++;
      delete req;
    }

    if( InitCount == NetId ){
      for( int i = 0; i < NumPeers; ++i ){
        req = new SimpleNetwork::Request(i,NetId,0,true,true);
        LinkControl->sendUntimedData(req);
      }
      InitState = 4;
    }
    break;
  case 4:
    SimpleNetwork::Request *req;
    while( (req = LinkControl->recvUntimedData()) != NULL ){
      InitCount++;
      delete req;
    }
    if( InitCount == NumPeers ){
      InitState = 5;
    }
    break;
  default:
    break;
  }
}

bool RevNIC::ClockHandler(Cycle_t cycle){
  return true;
}

// EOF
