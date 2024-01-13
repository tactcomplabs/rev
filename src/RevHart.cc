//
// _RevHart_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "RevHart.h"
namespace SST::RevCPU{

void RevHart::AssignNIC(RevNicAPI *nic){
  // Make sure we don't already have a NIC
  if(NIC){
   output->fatal(CALL_INFO, -1, "Error: NIC already assigned to this hart.\n");
  }

  output->verbose(CALL_INFO, 2, 0, "Assigning NIC to hart %" PRIu32 "\n and overriding the message handler.\n", ID);
  NIC = nic;
  NIC->setMsgHandler(new SST::Event::Handler<RevHart>(this, &RevHart::NetworkMsgHandler));
}

void RevHart::GiveAccessToNIC(RevNicAPI *nic){
  if(NIC){
    output->fatal(CALL_INFO, -1, "Error: NIC already assigned to this hart.\n");
  }
  output->verbose(CALL_INFO, 2, 0, "Giving hart %" PRIu32 " access to NIC\n", ID);
  NIC = nic;
}

void RevHart::NetworkMsgHandler(Event *ev){
  RevPkt *event = static_cast<RevPkt*>(ev);
  output->verbose(CALL_INFO, 2, 0, "Received a packet from the network for Hart %" PRIu32 "\n", ID);
  delete event;
  return;
}

} // namespace SST::RevCPU

