//
// _RevNOC_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVNOC_H_
#define _SST_REVNOC_H_

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

enum class NOCAckFmt : uint64_t {
  MSGID = 0,
  SRCID = 1,
  TYPE = 2,
};

enum class NOCDataFmt : uint64_t {
  MSGID = 0,
  SRCID = 1,
  TYPE = 2,
};

enum class NOCInitFmt : uint64_t {
  SRC_LOGICAL_ID = 0,
  SRC_ENDPOINT_ID = 1,
  TYPE = 2,
};

//enum class NetworkNodeType : uint64_t {
//  PRIMARY_REV = 0,
//  SECONDARY_REV = 1,
//};

// Maybe don't need this?
//enum NOCDataType : uint64_t {
//  STRING = 0,
//  VECTOR = 1,
//};
//
// NOTE: this probably doesn't need to be a uint64_t
enum class RevNOCPacketType : uint64_t {
  INIT_BCAST = 0,
  ACK = 1,
  DATA = 2,
  TERM = 3,
  TERM_ACK = 4, // Need separate ACK for this to prevent deadlock
};

/**
 * RevNOCPkt : inherited class to handle the individual network events for RevNOC
 **/
class RevNOCPkt : public SST::Event{
public:
  /// RevNOCPkt: extended constructor
  RevNOCPkt(RevNOCPacketType Type, uint64_t Src, std::vector<uint64_t> Data)
    : Event(), Type(Type), SrcLogicalID(Src), Data(std::move(Data)) {

    // create a random message ID
    MsgID = RevRand(0, UINT64_MAX);
  }

  // RevNOCPkt: retrieve the data payload
  std::vector<uint64_t> getData() { return Data; }

  // RevNOCPkt: set the data payload
  void setData(std::vector<uint64_t> D){ Data = D; }

  // RevNOCPkt: retrieve the packet type
  RevNOCPacketType& getType() { return Type; }

  // RevNOCPkt: set the packet type
  void setType(RevNOCPacketType T){ Type = T; }

  // RevNOCPkt: retrieve the source logical ID
  uint64_t getSrc() { return SrcLogicalID; }

  // RevNOCPkt: Get the message ID
  uint64_t getMsgID(){ return MsgID; }

  /// RevNOCPkt: virtual function to clone an event
  virtual Event* clone(void) override{
    RevNOCPkt* ev = new RevNOCPkt(*this);
    return ev;
  }

private:
  RevNOCPacketType Type;                ///< RevNOCPkt: Packet type
  uint64_t SrcLogicalID;          ///< RevNOCPkt: Source logical ID
  uint64_t MsgID;                 ///< RevNOCPkt: Packet type
  std::vector<uint64_t> Data;     ///< RevNOCPkt: Data payload

public:
  /// RevNOCPkt: secondary constructor
  RevNOCPkt() : Event() {}

  /// RevNOCPkt: event serializer
  void serialize_order(SST::Core::Serialization::serializer &ser) override {
    Event::serialize_order(ser);
    ser & Type;
    ser & SrcLogicalID;
    ser & MsgID;
    ser & Data;
  }

  /// RevNOCPkt: implements the NOC serialization
  ImplementSerializable(SST::RevCPU::RevNOCPkt);
};  // end RevNOCPkt


/**
 * RevNocAPI : Handles the subcomponent NOC API
 */
class RevNocAPI: public SST::SubComponent {
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::RevNocAPI)

  SST_ELI_DOCUMENT_PARAMS({ "verbose", "Set the verbosity of output for the memory controller", "0" } )

  /// RevNocAPI: constructor
  RevNocAPI(ComponentId_t id, Params& params) : SubComponent(id) { }

  /// RevNocAPI: default destructor
  virtual ~RevNocAPI() = default;

  /// RevNocAPI: registers the event handler with the core
  virtual void setMsgHandler(Event::HandlerBase* handler) = 0;

  /// RevNocAPI: initializes the network
  virtual void init(unsigned int phase) = 0;

  /// RevNocAPI: setup the network
  virtual void setup() { }

  /// RevNocAPI: get the source logical ID
  virtual uint64_t GetLogicalID() = 0;

  /// RevNocAPI: set the source logical ID
  virtual void SetLogicalID(uint64_t logicalID) = 0;

  /// RevNocAPI: send a message on the network
  virtual void send(RevNOCPkt *pkt, uint64_t logicalDestID) = 0;

  // TODO: Comment
  virtual void sendString(const std::string& str, RevNOCPacketType Type, uint64_t dest) = 0;

  /// RevNocAPI: retrieve the number of potential destinations
  virtual uint64_t getNumDestinations() = 0;

  /// RevNocAPI: returns the NOC's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress() = 0;

  // RevNocAPI: returns the number of items in the send queue
  virtual unsigned getNumOutstanding() = 0;

  virtual std::vector<uint64_t> StringToVecU64(const std::string& str) = 0;

  virtual void ProcessAck(RevNOCPkt* pkt) = 0;

  virtual void AckThisPkt(const uint64_t MsgID, const uint64_t DestLogicalID) = 0;

  virtual size_t GetNumUnackedPkts() = 0;

  virtual std::string VecU64ToString(const std::vector<uint64_t>& vec) = 0;

}; /// end RevNocAPI

/**
 * RevNOC: the Rev network interface controller subcomponent
 */
class RevNOC : public RevNocAPI {
public:

  // Register with the SST Core
  SST_ELI_REGISTER_SUBCOMPONENT(
    RevNOC,
    "revcpu",
    "RevNOC",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "RISC-V SST NOC",
    SST::RevCPU::RevNocAPI
    )

  // Register the parameters
  SST_ELI_DOCUMENT_PARAMS(
    {"clock", "Clock frequency of the NOC", "1Ghz"},
    {"port", "Port to use, if loaded as an anonymous subcomponent", "network"},
    {"verbose", "Verbosity for output (0 = nothing)", "0"},
    )

  // Register the ports
  SST_ELI_DOCUMENT_PORTS(
    {"network", "Port to network", {"simpleNetworkExample.nocEvent"} }
    )

  // Register the subcomponent slots
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"iface", "SimpleNetwork interface to a network", "SST::Interfaces::SimpleNetwork"}
    )

  /// RevNOC: constructor
  RevNOC(ComponentId_t id, Params& params);

  /// RevNOC: destructor
  virtual ~RevNOC();

  /// RevNOC: Callback to parent on received messages
  virtual void setMsgHandler(Event::HandlerBase* handler);

  /// RevNOC: initialization function
  virtual void init(unsigned int phase);

  /// RevNOC: setup function
  virtual void setup();

  /// RevNOC: set the source ID
  virtual uint64_t GetLogicalID(){ return LogicalID; }

  /// RevNOC: set the source ID
  virtual void SetLogicalID(uint64_t logicalID){ LogicalID = logicalID; }

  /// RevNOC: send event to the destination id
  virtual void send(RevNOCPkt *pkt, uint64_t logicalDestID);

  // TODO: Comment
  virtual void sendString(const std::string& str, RevNOCPacketType Type, uint64_t dest);

  /// RevNOC: retrieve the number of destinations
  virtual uint64_t getNumDestinations();

  /// RevNOC: get the endpoint's network address
  virtual SST::Interfaces::SimpleNetwork::nid_t getAddress();

  /// RevNOC: callback functions for the SimpleNetwork interface
  bool msgRecvNotify(int virtualNetwork);

  // bool msgSendNotify(int virtualNetwork);

  /// RevNOC: clock function
  virtual bool ClockTick(Cycle_t cycle);

  /// RevNOC: returns the number of items in the send queue
  virtual unsigned getNumOutstanding() { return sendQ.size(); }

  /// RevNOC: sends ack to src of pkt
  virtual void AckThisPkt(const uint64_t msgID, const uint64_t srcID);

  /// RevNOC: convert a std::string to a std::vector<uint64_t> (used for sendString )
  virtual std::vector<uint64_t> StringToVecU64(const std::string& str);

  virtual std::string VecU64ToString(const std::vector<uint64_t>& vec);

  /// RevNOC: process the received ack (ie. remove from outstanding acks)
  virtual void ProcessAck(RevNOCPkt *pkt);

  /// RevNOC: return the number of unacked messages
  virtual size_t GetNumUnackedPkts() { return OutstandingAcks.size(); }

protected:
  SST::Output *output;                    ///< RevNOC: SST output object

  SST::Interfaces::SimpleNetwork * iFace; ///< RevNOC: SST network interface

  SST::Event::HandlerBase *msgHandler;    ///< RevNOC: SST message handler

  bool initBroadcastSent;                 ///< RevNOC: has the init bcast been sent?

  uint64_t numDest;                       ///< RevNOC: number of SST destinations

  uint64_t LogicalID;                            ///< RevNOC: logical source ID

  std::queue<SST::Interfaces::SimpleNetwork::Request*> sendQ; ///< RevNOC: buffered send queue

  std::map<uint64_t,SST::Interfaces::SimpleNetwork::nid_t> hostMap;  ///< RevNOC: host map

  std::set<uint64_t> OutstandingAcks;      ///< RevNOC: outstanding acks

}; // end RevNOC

} // namespace SST::RevCPU

#endif // _SST_REVNOC_H_

// EOF
