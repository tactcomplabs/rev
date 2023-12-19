//
// _RevNIC_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVNIC_H_
#define _SST_REVNIC_H_

// -- Standard Headers
#include <vector>
#include <queue>
#include <tuple>
#include <utility>
#include <string>
#include <unistd.h>

// -- SST Headers
#include "SST.h"

namespace SST::RevCPU{

/**
 * nicEvent : inherited class to handle the individual network events for RevNIC
 */
class nicEvent : public SST::Event{
public:
  /// nicEvent: extended constructor
  nicEvent(std::vector<uint64_t> data)
    : Event(), Data(std::move(data)) { }

  // nicEvent: retrieve the data payload
  std::vector<uint64_t> getData() { return Data; }

  // nicEvent: set the data payload
  void setData(std::vector<uint64_t> D){ Data = D; }

  /// nicEvent: virtual function to clone an event
  virtual Event* clone(void) override{
    nicEvent* ev = new nicEvent(*this);
    return ev;
  }

private:
  std::vector<uint64_t> Data;     ///< nicEvent: Data payload

public:
  /// nicEvent: secondary constructor
  nicEvent() : Event() {}

  /// nicEvent: event serializer
  void serialize_order(SST::Core::Serialization::serializer &ser) override{
    Event::serialize_order(ser);
    ser & Data;
  }

  /// nicEvent: implements the NIC serialization
  ImplementSerializable(SST::RevCPU::nicEvent);
};  // end nicEvent


/**
 * nicAPI : Handles the subcomponent NIC API
 */
class nicAPI: public SST::SubComponent{
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::nicAPI)

  /// nicEvent: constructor
  nicAPI(ComponentId_t id, Params& params) : SubComponent(id) { }

  /// nicEvent: default destructor
  virtual ~nicAPI() = default;

  /// nicEvent: registers the event handler with the core
  virtual void setMsgHandler(Event::HandlerBase* handler) = 0;

  /// nicEvent: initializes the network
  virtual void init(unsigned int phase) = 0;

  /// nicEvent: setup the network
  virtual void setup() { }

  /// nicEvent: set the source logical ID
  virtual void setID(uint64_t ID) = 0;

  /// nicEvent: send a message on the network
  virtual void send(nicEvent *ev, uint64_t dest) = 0;

  /// nicEvent: retrieve the number of potential destinations
  virtual uint64_t getNumDestinations() = 0;

  /// nicEvent: returns the NIC's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress() = 0;
}; /// end nicAPI

/**
 * RevNIC: the Rev network interface controller subcomponent
 */
class RevNIC : public nicAPI {
public:

  // Register with the SST Core
  SST_ELI_REGISTER_SUBCOMPONENT(
    RevNIC,
    "revcpu",
    "RevNIC",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "RISC-V SST NIC",
    SST::RevCPU::nicAPI
    )

  // Register the parameters
  SST_ELI_DOCUMENT_PARAMS(
    {"clock", "Clock frequency of the NIC", "1Ghz"},
    {"port", "Port to use, if loaded as an anonymous subcomponent", "network"},
    {"verbose", "Verbosity for output (0 = nothing)", "0"},
    )

  // Register the ports
  SST_ELI_DOCUMENT_PORTS(
    {"network", "Port to network", {"simpleNetworkExample.nicEvent"} }
    )

  // Register the subcomponent slots
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"iface", "SimpleNetwork interface to a network", "SST::Interfaces::SimpleNetwork"}
    )

  /// RevNIC: constructor
  RevNIC(ComponentId_t id, Params& params);

  /// RevNIC: destructor
  virtual ~RevNIC();

  /// RevNIC: Callback to parent on received messages
  virtual void setMsgHandler(Event::HandlerBase* handler);

  /// RevNIC: initialization function
  virtual void init(unsigned int phase);

  /// RevNIC: setup function
  virtual void setup();

  /// RevNIC: set the source ID
  virtual void setID(uint64_t I){ ID = I; }

  /// RevNIC: send event to the destination id
  virtual void send(nicEvent *ev, uint64_t dest);

  /// RevNIC: retrieve the number of destinations
  virtual uint64_t getNumDestinations();

  /// RevNIC: get the endpoint's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress();

  /// RevNIC: callback function for the SimpleNetwork interface
  bool msgNotify(int virtualNetwork);

  /// RevNIC: clock function
  virtual bool clockTick(Cycle_t cycle);

protected:
  SST::Output *output;                    ///< RevNIC: SST output object

  SST::Interfaces::SimpleNetwork * iFace; ///< RevNIC: SST network interface

  SST::Event::HandlerBase *msgHandler;    ///< RevNIC: SST message handler

  bool initBroadcastSent;                 ///< RevNIC: has the init bcast been sent?

  uint64_t numDest;                       ///< RevNIC: number of SST destinations

  uint64_t ID;                            ///< RevNIC: logical source ID

  std::queue<SST::Interfaces::SimpleNetwork::Request*> sendQ; ///< RevNIC: buffered send queue

  std::map<uint64_t,SST::Interfaces::SimpleNetwork::nid_t> hostMap;  ///< RevNIC: host map

}; // end RevNIC

} // namespace SST::RevCPU

#endif // _SST_REVNIC_H_

// EOF
