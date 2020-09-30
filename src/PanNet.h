//
// _PanNet_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_PANNET_H_
#define _SST_PANNET_H_

// -- Standard Headers
#include <vector>
#include <queue>
#include <unistd.h>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/interfaces/simpleNetwork.h>

// -- Rev Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevLoader.h"
#include "RevProc.h"

namespace SST {
  namespace RevCPU {
    /**
     * panNicEvent : inherited class to handle the individual network events for PanNet
     */
    class panNicEvent : public SST::Event {
    public:
      /// panNicEvent: standard constructor
      panNicEvent(std::string name) : Event(), SrcName(name) { }

      /// panNicEvent: rerieve the source name
      std::string getSource() { return SrcName; }

      /// panNicEvent: vritual function to clone an event
      virtual Event* clone(void) override{
        panNicEvent *ev = new panNicEvent(*this);
        return ev;
      }

    private:
      std::string SrcName;            ///< panNicEvent: Name of the sending device

    public:
      /// panNicEvent: event serializer
      panNicEvent() : Event() {}

      /// panNicEvent: event serializer
      void serialize_order(SST::Core::Serialization::serializer &ser) override{
        Event::serialize_order(ser);
        ser & SrcName;
      }

      /// panNicevent: implements the NIC serialization
      ImplementSerializable(SST::RevCPU::panNicEvent);
    }; // end panNicEvent


    /**
     * panNicAPI: Handles the subcomponent NIC API
     */
    class panNicAPI: public SST::SubComponent{
    public:
      SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::panNicAPI)

      /// panNicAPI: default constructor
      panNicAPI(ComponentId_t id, Params& params) : SubComponent(id) { }

      /// panNicAPI: default destructor
      virtual ~panNicAPI() { }

      /// panNicAPI: registers the event handler with the core
      virtual void setMsgHandler(Event::HandlerBase* handler) = 0;

      /// panNicAPI: initializes the network
      virtual void init(unsigned int phase) = 0;

      /// panNicAPI: setup the network
      virtual void setup() { }

      /// panNicAPI: send a message on the network
      virtual void send(panNicEvent *ev, int dest) = 0;

      /// panNicAPI: retrieve the number of potential destinations
      virtual int getNumDestinations() = 0;

      /// panNicAPI: returns the NIC's network address
       virtual SST::Interfaces::SimpleNetwork::nid_t getAddress() = 0;
    }; // end panNicAPI

    /**
     * PanNet: the PAN network transport module subcomponent
     */
    class PanNet : public panNicAPI {
    public:
      // Register with the SST Core
      SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(
        PanNet,
        "revcpu",
        "PanNet",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "PAN Network Transport Module",
        SST::RevCPU::panNicAPI
      )

      // Register the parameters
      SST_ELI_DOCUMENT_PARAMS(
        {"port", "Port to use, if loaded as an anonymous subcomponent", "network"},
        {"verbose", "Verbosity for output (0 = nothing)", "0"}
      )

      // Register the ports
      SST_ELI_DOCUMENT_PORTS(
        {"network", "Port to network", {"simpleNetworkExample.nicEvent"} }
      )

      // Register the subcomponent slots
      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
        {"iface", "SimpleNetwork interface to a network", "SST::Interfaces::SimpleNetwork"}
      )

      /// PanNet: default constructor
      PanNet(ComponentId_t id, Params& params);

      /// PanNet: default destructor
      virtual ~PanNet();

      /// PanNet: Callback to parent on received messages
      virtual void setMsgHandler(Event::HandlerBase* handler);

      /// PanNet: initialization function
      virtual void init(unsigned int phase);

      /// PanNet: setup function
      virtual void setup();

      /// PanNet: send the event to the destination id
      virtual void send(panNicEvent *ev, int dest);

      /// PanNet: retrieve the number of destinations
      virtual int getNumDestinations();

      /// PanNet: get the endpoint's network address
      virtual SST::Interfaces::SimpleNetwork::nid_t getAddress();

      /// PanNet: callback function for the SimpleNetwork itnerface
      bool msgNotify(int virtualNetwork);

      /// PanNet: clock function
      virtual bool clockTick(Cycle_t cycle);

    protected:
      SST::Output *output;                    ///< PanNet: SST output object

      SST::Interfaces::SimpleNetwork * iFace; ///< PanNet: SST network interface

      SST::Event::HandlerBase *msgHandler;    ///< PanNet: SST message handler

      bool initBroadcastSent;                 ///< PanNet: has the init bcast been sent?

      int numDest;                            ///< PanNet: number of SST destinations

      std::queue<SST::Interfaces::SimpleNetwork::Request*> sendQ; ///< PanNet: buffered send queue

    private:
      uint32_t Token;                         ///< PanNet: Reservation token

    }; // end PanNet

  } // namespace RevCPU
} // namespace SST

#endif

// EOF
