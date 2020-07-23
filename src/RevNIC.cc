//
// _RevNIC_cc_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevNIC.h"

RevNIC::RevNIC( SST::ComponentId_t id, SST::Params& params )
  : SST::Component(id),
    PacketsSent(0), PacketsRecv(0), StalledCycles(0), ExpRecvCount(0),
    Done(false), Init(false), InitCount(0), InitBCastCount(0),
    output(Simulation::getSimulation()->getSimulationOutput()) {
}

RevNIC::~RevNIC(){
  delete link_control;
  delete [] NextSeq;
}

void RevNIC::finish(){
}

void RevNIC::setup(){
}

void RevNIC::complete(){
}

void RevNIC::init(unsigned int phase){
}

void RevNIC::InitComplete(unsigned int phase){
}

bool RevNIC::ClockHandler(Cycle_t cycle){
  return true;
}

// EOF
