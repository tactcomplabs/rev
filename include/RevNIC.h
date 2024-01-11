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

#include "RevRand.h"

namespace SST::RevCPU{

using namespace SST::Interfaces;

enum class AckFmt : uint64_t {
  MSGID = 0,
  SRCID = 1,
  TYPE = 2,
};

enum class DataFmt : uint64_t {
  MSGID = 0,
  SRCID = 1,
  TYPE = 2,
};

enum class InitFmt : uint64_t {
  SRC_LOGICAL_ID = 0,
  SRC_ENDPOINT_ID = 1,
  TYPE = 2,
};

enum class NetworkNodeType : uint64_t {
  PRIMARY_REV = 0,
  SECONDARY_REV = 1,
};

// Maybe don't need this?
enum DataType : uint64_t {
  STRING = 0,
  VECTOR = 1,
};

// NOTE: this probably doesn't need to be a uint64_t
enum class PacketType : uint64_t {
  INIT_BCAST = 0,
  ACK = 1,
  DATA = 2,
  TERM = 3,
  TERM_ACK = 4, // Need separate ACK for this to prevent deadlock
};

/**
 * RevPkt : inherited class to handle the individual network events for RevNIC
 **/
class RevPkt : public SST::Event{
public:
  /// RevPkt: extended constructor
  RevPkt(PacketType Type, uint64_t Src, std::vector<uint64_t> Data)
    : Event(), Type(Type), SrcLogicalID(Src), Data(std::move(Data)) {

    // create a random message ID
    MsgID = RevRand(0, UINT64_MAX);
  }

  // RevPkt: retrieve the data payload
  std::vector<uint64_t> getData() { return Data; }

  // RevPkt: set the data payload
  void setData(std::vector<uint64_t> D){ Data = D; }

  // RevPkt: retrieve the packet type
  PacketType& getType() { return Type; }

  // RevPkt: set the packet type
  void setType(PacketType T){ Type = T; }

  // RevPkt: retrieve the source logical ID
  uint64_t getSrc() { return SrcLogicalID; }

  // RevPkt: Get the message ID
  uint64_t getMsgID(){ return MsgID; }

  /// RevPkt: virtual function to clone an event
  virtual Event* clone(void) override{
    RevPkt* ev = new RevPkt(*this);
    return ev;
  }

private:
  PacketType Type;                ///< RevPkt: Packet type
  uint64_t SrcLogicalID;          ///< RevPkt: Source logical ID
  uint64_t MsgID;                 ///< RevPkt: Packet type
  std::vector<uint64_t> Data;     ///< RevPkt: Data payload

public:
  /// RevPkt: secondary constructor
  RevPkt() : Event() {}

  /// RevPkt: event serializer
  void serialize_order(SST::Core::Serialization::serializer &ser) override {
    Event::serialize_order(ser);
    ser & Type;
    ser & SrcLogicalID;
    ser & MsgID;
    ser & Data;
  }

  /// RevPkt: implements the NIC serialization
  ImplementSerializable(SST::RevCPU::RevPkt);
};  // end RevPkt


/**
 * RevNicAPI : Handles the subcomponent NIC API
 */
class RevNicAPI: public SST::SubComponent {
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::RevNicAPI)

  SST_ELI_DOCUMENT_PARAMS({ "verbose", "Set the verbosity of output for the memory controller", "0" } )

  /// RevNicAPI: constructor
  RevNicAPI(ComponentId_t id, Params& params) : SubComponent(id) { }

  /// RevNicAPI: default destructor
  virtual ~RevNicAPI() = default;

  /// RevNicAPI: registers the event handler with the core
  virtual void setMsgHandler(Event::HandlerBase* handler) = 0;

  /// RevNicAPI: initializes the network
  virtual void init(unsigned int phase) = 0;

  /// RevNicAPI: setup the network
  virtual void setup() { }

  /// RevNicAPI: get the source logical ID
  virtual uint64_t GetLogicalID() = 0;

  /// RevNicAPI: set the source logical ID
  virtual void SetLogicalID(uint64_t logicalID) = 0;

  /// RevNicAPI: send a message on the network
  virtual void send(RevPkt *pkt, uint64_t logicalDestID) = 0;

  // TODO: Comment
  virtual void sendString(const std::string& str, PacketType Type, uint64_t dest) = 0;

  /// RevNicAPI: retrieve the number of potential destinations
  virtual uint64_t getNumDestinations() = 0;

  /// RevNicAPI: returns the NIC's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress() = 0;

  // RevNicAPI: returns the number of items in the send queue
  virtual unsigned getNumOutstanding() = 0;

  virtual std::vector<uint64_t> StringToVecU64(const std::string& str) = 0;

  virtual void ProcessAck(RevPkt* pkt) = 0;

  virtual void AckThisPkt(const uint64_t MsgID, const uint64_t DestLogicalID) = 0;

  virtual size_t GetNumUnackedPkts() = 0;

  virtual std::string VecU64ToString(const std::vector<uint64_t>& vec) = 0;

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
  virtual uint64_t GetLogicalID(){ return LogicalID; }

  /// RevNIC: set the source ID
  virtual void SetLogicalID(uint64_t logicalID){ LogicalID = logicalID; }

  /// RevNIC: send event to the destination id
  virtual void send(RevPkt *pkt, uint64_t logicalDestID);

  // TODO: Comment
  virtual void sendString(const std::string& str, PacketType Type, uint64_t dest);

  /// RevNIC: retrieve the number of destinations
  virtual uint64_t getNumDestinations();

  /// RevNIC: get the endpoint's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress();

  /// RevNIC: callback functions for the SimpleNetwork interface
  bool msgRecvNotify(int virtualNetwork);

  // bool msgSendNotify(int virtualNetwork);

  /// RevNIC: clock function
  virtual bool ClockTick(Cycle_t cycle);

  /// RevNIC: returns the number of items in the send queue
  virtual unsigned getNumOutstanding() { return sendQ.size(); }

  /// RevNIC: sends ack to src of pkt
  virtual void AckThisPkt(const uint64_t msgID, const uint64_t srcID);

  /// RevNIC: convert a std::string to a std::vector<uint64_t> (used for sendString )
  virtual std::vector<uint64_t> StringToVecU64(const std::string& str);

  virtual std::string VecU64ToString(const std::vector<uint64_t>& vec);

  /// RevNIC: process the received ack (ie. remove from outstanding acks)
  virtual void ProcessAck(RevPkt *pkt);

  /// RevNIC: return the number of unacked messages
  virtual size_t GetNumUnackedPkts() { return OutstandingAcks.size(); }

protected:
  SST::Output *output;                    ///< RevNIC: SST output object

  SST::Interfaces::SimpleNetwork * iFace; ///< RevNIC: SST network interface

  SST::Event::HandlerBase *msgHandler;    ///< RevNIC: SST message handler

  bool initBroadcastSent;                 ///< RevNIC: has the init bcast been sent?

  uint64_t numDest;                       ///< RevNIC: number of SST destinations

  uint64_t LogicalID;                            ///< RevNIC: logical source ID

  std::queue<SST::Interfaces::SimpleNetwork::Request*> sendQ; ///< RevNIC: buffered send queue

  std::map<uint64_t,SST::Interfaces::SimpleNetwork::nid_t> hostMap;  ///< RevNIC: host map

  std::set<uint64_t> OutstandingAcks;      ///< RevNIC: outstanding acks

}; // end RevNIC

} // namespace SST::RevCPU

#endif // _SST_REVNIC_H_

// EOF
