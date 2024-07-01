//
// _RevVer_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevVer.h"

namespace SST::RevCPU {

VerilatorCtrl::VerilatorCtrl( ComponentId_t id, Params& params ) : verilatorAPI( id, params ) {
  // setup the initial logging functions
  int verbosity                  = params.find<int>( "verbose", 0 );
  output                         = new SST::Output( "", verbosity, 0, SST::Output::STDOUT );

  const std::string verCtrlClock = params.find<std::string>( "clock", "1GHz" );
  registerClock( verCtrlClock, new Clock::Handler<VerilatorCtrl>( this, &VerilatorCtrl::clockTick ) );

  clkLink = configureLink( "clk", "0ns", new Event::Handler<RevCPU>( this, &RevCPU::RecvPortEvent ) );
  if( nullptr == clkLink ) {
    output.fatal( CALL_INFO, -1, "Error: was unable to configureLink %s \n", "clk" );
  }
  reset_lLink = configureLink( "reset_l", "0ns", new Event::Handler<RevCPU>( this, &RevCPU::RecvPortEvent ) );
  if( nullptr == reset_lLink ) {
    output.fatal( CALL_INFO, -1, "Error: was unable to configureLink %s \n", "reset_l" );
  }
  stopLink = configureLink( "stop", "0ns", new Event::Handler<RevCPU>( this, &RevCPU::RecvPortEvent ) );
  if( nullptr == stopLink ) {
    output.fatal( CALL_INFO, -1, "Error: was unable to configureLink %s \n", "stop" );
  }
  doneLink = configureLink( "done", "0ns", new Event::Handler<RevCPU>( this, &RevCPU::RecvPortEvent ) );
  if( nullptr == doneLink ) {
    output.fatal( CALL_INFO, -1, "Error: was unable to configureLink %s \n", "done" );
  }

  numDest    = 0;

  msgHandler = nullptr;
}

VerilatorCtrl::~VerilatorCtrl() {
  delete output;
}

void VerilatorCtrl::RecvPortEvent( SST::Event* ev ) {
  output.verbose( CALL_INFO, 1, 0, "Received an event\n" );
  // Receive an event... should only be receiving port events

SST:
  VerilatorSST::PortEvent* fromPort = static_cast < SST : VerilatorSST::PortEvent* > ( ev );

  if( fromPort ) {
    output.verbose( CALL_INFO, 1, 0, "Received a port event from verilatorSST\n" );

    // Create a new Ackevent
    //SimpleOS::AckEvent* ack = new SimpleOS::AckEvent( shutdown->getMsgID() );
    //OSLink->send( ack );
    //TODO: if we want to implement acks

    delete fromPort;
    return;
  }
  //SimpleOS::ThreadEvent* Thread = static_cast<SimpleOS::ThreadEvent*>( ev );
  //if( Thread ) {
  //  output.verbose( CALL_INFO, 1, 0, "Received a thread event from the OS\n" );
}

void VerilatorCtrl::setMsgHandler( Event::HandlerBase* handler ) {
  msgHandler = handler;
}

void VerilatorCtrl::init( unsigned int phase ) {
  /*
  iFace->init( phase );

  if( iFace->isNetworkInitialized() ) {
    if( !initBroadcastSent ) {
      initBroadcastSent                            = true;
      SST::VerilatorSST::PortEvent * ev            = new SST::VerilatorSST::PortEvent(  ); //TODO

      SST::Interfaces::SimpleNetwork::Request* req = new SST::Interfaces::SimpleNetwork::Request();
      req->dest                                    = SST::Interfaces::SimpleNetwork::INIT_BROADCAST_ADDR;
      req->src                                     = iFace->getEndpointID();
      req->givePayload( ev );

      //iFace->sendInitData( req );  // removed for SST 14.0.0
      iFace->sendUntimedData( req );
    }
  }
  //while( SST::Interfaces::SimpleNetwork::Request* req = iFace->recvInitData() ) {
  while( SST::Interfaces::SimpleNetwork::Request* req = iFace->recvUntimedData() ) {  // SST 14.0.0
    SST::VerilatorSST::PortEvent* ev = static_cast<SST::VerilatorSST::PortEvent*>( req->takePayload() ); //TODO
    numDest++;
    output->verbose( CALL_INFO, 1, 0, "%s received init message from %lx\n", getName().c_str(), ev->getAtTick() ); //TODO
  }
  */
}

void VerilatorCtrl::setup() {
  if( msgHandler == nullptr ) {
    output->fatal(
      CALL_INFO,
      -1,
      "%s, Error: VerilatorCtrl implements a callback-based notification "
      "and parent has not registerd a callback function\n",
      getName().c_str()
    );
  }
}

/*
void VerilatorCtrl::send( SST::VerilatorSST::PortEvent* event, int destination ) {
  SST::Interfaces::SimpleNetwork::Request* req = new SST::Interfaces::SimpleNetwork::Request(); //TODO
  req->dest                                    = destination;
  req->src                                     = iFace->getEndpointID();
  req->givePayload( event );
  sendQ.push( req );
}

int VerilatorCtrl::getNumDestinations() {
  return numDest;
}
*/

bool VerilatorCtrl::clockTick( Cycle_t cycle ) {
  /*
  while( !sendQ.empty() ) {
    if( iFace->spaceToSend( 0, 512 ) && iFace->send( sendQ.front(), 0 ) ) {
      sendQ.pop();
    } else {
      break;
    }
  }
  */

  return false;
}

}  // namespace SST::RevCPU

// EOF
