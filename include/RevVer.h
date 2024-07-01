//
// _RevVer_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVVER_H_
#define _SST_REVVER_H_

// -- Standard Headers
#include <queue>
#include <string>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>

// -- SST Headers
#include "SST.h"

namespace SST::VerilatorSST {

/**
 * PortEvent : inherited class to handle events between verilatorSST and Rev
 */
class PortEvent : public SST::Event {
public:
  /// PortEvent: default constructor
  explicit PortEvent() : Event(), AtTick( 0x00ull ) {}

  /// PortEvent: overloaded constructor taking target tick
  explicit PortEvent( uint64_t Tick ) : Event(), AtTick( Tick ) {}

  /// PortEvent: overloaded constructor taking data payload
  explicit PortEvent( std::vector<uint8_t> P ) : Event(), AtTick( 0x00ull ) {
    std::copy( P.begin(), P.end(), std::back_inserter( Packet ) );
  }

  /// PortEvent: overloaded constructor taking data payload and target tick
  explicit PortEvent( std::vector<uint8_t> P, uint64_t Tick ) : Event(), AtTick( Tick ) {
    std::copy( P.begin(), P.end(), std::back_inserter( Packet ) );
  }

  /// PortEvent: virtual clone function
  virtual Event* clone( void ) override {
    PortEvent* pe = new PortEvent( *this );
    return pe;
  }

  /// PortEvent: retrieve the target clock tick
  uint64_t getAtTick() { return AtTick; }

  /// PortEvent: retrieve the packet payload
  const std::vector<uint8_t> getPacket() { return Packet; }

  /// PortEvent: set the target clock tick
  void setAtTick( uint64_t T ) { AtTick = T; }

  /// PortEvent: set the payload
  void setPayload( const std::vector<uint8_t> P ) { std::copy( P.begin(), P.end(), std::back_inserter( Packet ) ); }

private:
  std::vector<uint8_t> Packet;  /// event packet
  uint64_t             AtTick;  /// event at clock tick

public:
  // PortEvent: event serializer
  void serialize_order( SST::Core::Serialization::serializer& ser ) override {
    // we only serialize the raw packet
    Event::serialize_order( ser );
    ser& Packet;
    ser& AtTick;
  }

  // PortEvent: implements the nic serialization
  ImplementSerializable( SST::VerilatorSST::PortEvent );
};  // end PortEvent

}  // namespace SST::VerilatorSST

namespace SST::RevCPU {

/**
 * VerilatorAPI : Handles the subcomponent Verilator API
 */
class verilatorAPI : public SST::SubComponent {
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API( SST::RevCPU::verilatorAPI )

  /// verilatorEvent: constructor
  verilatorAPI( ComponentId_t id, Params& params ) : SubComponent( id ) {}

  /// verilatorEvent: default destructor
  virtual ~verilatorAPI()                                   = default;

  /// verilatorEvent: registers the event handler with the core
  virtual void setMsgHandler( Event::HandlerBase* handler ) = 0;

  /// verilatorEvent: initializes the network
  virtual void init( unsigned int phase )                   = 0;

  /// verilatorEvent: setup the network
  virtual void setup() {}

  /// verilatorEvent: send a message on the network
  //virtual void send( SST::VerilatorSST::PortEvent* ev, int dest )         = 0;

  /// verilatorEvent: returns the NIC's network address
  //virtual SST::Interfaces::SimpleNetwork::nid_t getAddress()= 0;
};  /// end verilatorAPI

/**
 * VerilatorCtrl: the Rev verilator controller subcomponent
 */
class VerilatorCtrl : public verilatorAPI {
public:
  // Register with the SST Core
  SST_ELI_REGISTER_SUBCOMPONENT(
    VerilatorCtrl,
    "revcpu",
    "VerilatorCtrl",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "RISC-V driven verilator-sst controller",
    SST::RevCPU::verilatorAPI
  )

  // Register the parameters
  SST_ELI_DOCUMENT_PARAMS(
    { "clock", "Clock frequency of the controller", "1Ghz" },
    { "port", "Port to use, if loaded as an anonymous subcomponent", "network" },  //TODO
    { "verbose", "Verbosity for output (0 = nothing)", "0" },
  )

  // Register the ports
  SST_ELI_DOCUMENT_PORTS(
    { "clk", "clock port", { "" } },  // TODO: should clk port also be limited to PortEvent or driven by the subcomponent's clock?
    { "reset_l", "reset port (active low)", { "SST::VerilatorSST::PortEvent" } },
    { "stop", "port to set value to count to", { "SST::VerilatorSST::PortEvent" } },
    { "done", "port that is high when counting is complete", { "SST::VerilatorSST::PortEvent" } }
  )

  // Register the subcomponent slots
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()  //TODO

  /// VerilatorCtrl: constructor
  VerilatorCtrl( ComponentId_t id, Params& params );

  /// VerilatorCtrl: destructor
  virtual ~VerilatorCtrl();

  /// VerilatorCtrl: Callback to parent on received messages
  virtual void setMsgHandler( Event::HandlerBase* handler );

  /// VerilatorCtrl: initialization function
  virtual void init( unsigned int phase );

  /// VerilatorCtrl: setup function
  virtual void setup();

  /// VerilatorCtrl: send event to the destination id
  //virtual void send( SST::VerilatorSST::PortEvent* ev, int dest );

  /// VerilatorCtrl: clock function
  virtual bool clockTick( Cycle_t cycle );

protected:
  SST::Output*             output{};      ///< VerilatorCtrl: SST output object
  SST::Event::HandlerBase* msgHandler{};  ///< VerilatorCtrl: SST message handler
  //bool                            initBroadcastSent{};  ///< VerilatorCtrl: has the init bcast been sent? //TODO
  //int                             numDest{};            ///< VerilatorCtrl: number of SST destinations

  std::queue<SST::Interfaces::SimpleNetwork::Request*> sendQ{};  ///< VerilatorCtrl: buffered send queue //TODO

  SST::Link* clkLink;
  SST::Link* reset_lLink;
  SST::Link* stopLink;
  SST::Link* doneLink;

  /// VerilatorCtrl: disallow copying and assignment
  VerilatorCtrl( const VerilatorCtrl& )            = delete;
  VerilatorCtrl& operator=( const VerilatorCtrl& ) = delete;
};  // end VerilatorCtrl

}  // namespace SST::RevCPU

#endif  // _SST_REVVER_H_

        // EOF
