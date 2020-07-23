//
// _RevNIC_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVNIC_H_
#define _SST_REVNIC_H_

// -- Standard Headers
#include <vector>
#include <unistd.h>

// -- SST Headers
#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/interfaces/simpleNetwork.h>

namespace SST::RevNIC {
  class RevNIC;
}

using namespace SST::Interfaces;
using namespace SST::RevNIC;

namespace SST {
  namespace RevNIC {
    class RevNIC : public SST::Component {

    public:
      /// RevNIC: Rev network interface controller SST component constructor
      RevNIC( SST::ComponentId_t id, SST::Params& params );

      /// RevNIC: Rev network interface controller SST component destructor
      ~RevNIC();

      /// RevNIC: Rev network interface controller SST component 'setup' function
      void setup();

      /// RevNIC: Rev network interface controller SST component 'finish' function
      void finish();

      /// RevNIC: Rev network inteface controller SST component 'complete' function
      void complete(unsigned int phase);

      /// RevNIC: Rev network interface controller SST component 'init' function
      void init( unsigned int phase );

      // -------------------------------------------------------
      // RevNIC Component Registration Data
      // -------------------------------------------------------
      /// RevNIC: Register the component with the SST core
      SST_ELI_REGISTER_COMPONENT(
                                  RevNIC,
                                  "revcpu",
                                  "RevNIC",
                                  SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                                  "RISC-V Rev Network Interface Controller",
                                  COMPONENT_CATEGORY_NETWORK
                                )

      // -------------------------------------------------------
      // RevNIC Component Parameter Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_PARAMS(
                              {"id",           "Network ID of endpoint."},
                              {"num_peers",    "Total number of endpoints in network."},
                              {"num_messages", "Total number of messages to send to each endpoint."},
                              {"message_size", "Size of each message to be sent specified in either b or B (can include SI prefix)."},
                              {"send_untimed_broadcast",   "Controls whether data is sent in init and complete.","false"}
                             )

      // -------------------------------------------------------
      // RevNIC Port Parameter Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_PORTS(
                            )

      // -------------------------------------------------------
      // RevNIC SubComponent Parameter Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
        {"networkIF", "Network interface", "SST::Interfaces::SimpleNetwork" }
      )

      // -------------------------------------------------------
      // RevNIC Component Statistics Data
      // -------------------------------------------------------
      SST_ELI_DOCUMENT_STATISTICS(
                                 )

    private:
      int id;               ///< RevNIC: SST::Interfaces::SimpleNetwork::nid_t identifier
      int NetId;            ///< RevNIC: network id
      int NumPeers;         ///< RevNIC: number of network peers
      int MsgSize;          ///< RevNIC: message size
      int NumMsg;           ///< RevNIC: number of messages
      int GroupOffset;      ///< RevNIC: Group offset
      int GroupPeers;       ///< RevNIC: Group peers

      int PacketsSent;      ///< RevNIC: Number of packets sent
      int PacketsRecv;      ///< RevNIC: Number of packets received
      int StalledCycles;    ///< RevNIC: Number of stalled cycles
      int ExpRecvCount;     ///< RevNIC: Expected receive count

      bool Done;            ///< RevNIC: Completion signal
      bool Init;            ///< RevNIC: Signals initialization complete
      int InitState;        ///< RevNIC: State initialization
      int InitCount;        ///< RevNIC: Count initialization
      int InitBCastCount;   ///< RevNIC: Broadcast count initialization
      bool SendUntimedBCast;///< RevNIC: Flags to enable untimed broadcast messages

      int LastTarget;       ///< RevNIC: Last target id
      int *NextSeq;         ///< RevNIC: Next sequence pointer

      SST::Output &output;  ///< RevNIC: SST output handler

      SST::Interfaces::SimpleNetwork *LinkControl;   ///< RevNIC: SST link control object

      /// RevNIC: Clock handler
      bool ClockHandler(Cycle_t cycle);

      /// RevNIC: Initialization completion
      void InitComplete(unsigned int phase);

    }; // class RevNIC
  } // namespace RevNIC
} // namespace SST

#endif // _SST_REVNIC_H_

// EOF
