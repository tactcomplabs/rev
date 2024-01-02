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
 * RevPkt : inherited class to handle the individual network events for RevNIC
 */
class RevPkt : public SST::Event{
public:
  /// RevPkt: extended constructor
  RevPkt(std::vector<uint64_t> data)
    : Event(), Data(std::move(data)) { }

  // RevPkt: retrieve the data payload
  std::vector<uint64_t> getData() { return Data; }

  // RevPkt: set the data payload
  void setData(std::vector<uint64_t> D){ Data = D; }

  /// RevPkt: virtual function to clone an event
  virtual Event* clone(void) override{
    RevPkt* ev = new RevPkt(*this);
    return ev;
  }

private:
  std::vector<uint64_t> Data;     ///< RevPkt: Data payload

public:
  /// RevPkt: secondary constructor
  RevPkt() : Event() {}

  /// RevPkt: event serializer
  void serialize_order(SST::Core::Serialization::serializer &ser) override{
    Event::serialize_order(ser);
    ser & Data;
  }

  /// RevPkt: implements the NIC serialization
  ImplementSerializable(SST::RevCPU::RevPkt);
};  // end RevPkt


/**
 * RevNicAPI : Handles the subcomponent NIC API
 */
class RevNicAPI: public SST::SubComponent{
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::RevNicAPI)

  /// RevPkt: constructor
  RevNicAPI(ComponentId_t id, Params& params) : SubComponent(id) { }

  /// RevPkt: default destructor
  virtual ~RevNicAPI() = default;

  /// RevPkt: registers the event handler with the core
  virtual void setMsgHandler(Event::HandlerBase* handler) = 0;

  /// RevPkt: initializes the network
  virtual void init(unsigned int phase) = 0;

  /// RevPkt: setup the network
  virtual void setup() { }

  /// RevPkt: set the source logical ID
  virtual void setID(uint64_t ID) = 0;

  /// RevPkt: send a message on the network
  virtual void send(RevPkt *ev, uint64_t dest) = 0;

  /// RevPkt: retrieve the number of potential destinations
  virtual uint64_t getNumDestinations() = 0;

  /// RevPkt: returns the NIC's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress() = 0;

  // RevPkt: returns the number of items in the send queue
  virtual unsigned getNumOutstanding() = 0;
}; /// end RevNicAPI

/**
 * RevNIC: the Rev network interface controller subcomponent
 */
class RevNIC : public RevNicAPI {
public:

  // Register with the SST Core
  SST_ELI_REGISTER_SUBCOMPONENT(
    RevNIC,
    "revcpu",
    "RevNIC",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "RISC-V SST NIC",
    SST::RevCPU::RevNicAPI
    )

  // Register the parameters
  SST_ELI_DOCUMENT_PARAMS(
    {"clock", "Clock frequency of the NIC", "1Ghz"},
    {"port", "Port to use, if loaded as an anonymous subcomponent", "network"},
    {"verbose", "Verbosity for output (0 = nothing)", "0"},
    )

  // Register the ports
  SST_ELI_DOCUMENT_PORTS(
    {"network", "Port to network", {"simpleNetworkExample.RevPkt"} }
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
  virtual void send(RevPkt *ev, uint64_t dest);

  /// RevNIC: retrieve the number of destinations
  virtual uint64_t getNumDestinations();

  /// RevNIC: get the endpoint's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress();

  /// RevNIC: callback function for the SimpleNetwork interface
  bool msgNotify(int virtualNetwork);

  /// RevNIC: clock function
  virtual bool clockTick(Cycle_t cycle);

  // RevPkt: returns the number of items in the send queue
  virtual unsigned getNumOutstanding() { return sendQ.size(); }

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
