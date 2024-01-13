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
  NIC->setMsgHandler(new SST::Event::Handler<RevHart>(this, &RevHart::NICMsgHandler));
}

void RevHart::GiveAccessToNIC(RevNicAPI *nic){
  if(NIC){
    output->fatal(CALL_INFO, -1, "Error: NIC already assigned to this hart.\n");
  }
  output->verbose(CALL_INFO, 2, 0, "Giving hart %" PRIu32 " access to NIC\n", ID);
  NIC = nic;
}

void RevHart::NICMsgHandler(Event *ev){
  RevPkt *event = static_cast<RevPkt*>(ev);
  output->verbose(CALL_INFO, 2, 0, "Received a packet from the network for Hart %" PRIu32 "\n", ID);
  delete event;
  return;
}

void RevHart::AssignNOC(RevNocAPI *noc){
  // Make sure we don't already have a NOC
  if(NOC){
   output->fatal(CALL_INFO, -1, "Error: NOC interface already assigned to this hart.\n");
  }

  output->verbose(CALL_INFO, 2, 0, "Assigning NOC interface to hart %" PRIu32 "\n and overriding the message handler.\n", ID);
  NOC = noc;
  NOC->setMsgHandler(new SST::Event::Handler<RevHart>(this, &RevHart::NICMsgHandler));
}

void RevHart::GiveAccessToNOC(RevNocAPI *noc){
  if(NOC){
    output->fatal(CALL_INFO, -1, "Error: NOC interface already assigned to this hart.\n");
  }
  output->verbose(CALL_INFO, 2, 0, "Giving hart %" PRIu32 " access to NOC interface\n", ID);
  NOC = noc;
}

void RevHart::NOCMsgHandler(Event *ev){
  RevNOCPkt *event = static_cast<RevNOCPkt*>(ev);
  output->verbose(CALL_INFO, 2, 0, "Received a packet from the NOC on Hart %" PRIu32 "\n", ID);
  delete event;
  return;
}


} // namespace SST::RevCPU

