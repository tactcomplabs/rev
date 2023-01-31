//
// _RevMemCtrl_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevMemCtrl.h"

using namespace SST;
using namespace RevCPU;

// ---------------------------------------------------------------
// RevMemCtrl
// ---------------------------------------------------------------
RevMemCtrl::RevMemCtrl(ComponentId_t id, Params& params)
  : SubComponent(id), output(nullptr) {

  uint32_t verbosity = params.find<uint32_t>("verbose");
  output = new SST::Output("[RevMemCtrl @t]: ", verbosity, 0, SST::Output::STDOUT);

}

RevMemCtrl::~RevMemCtrl(){
  delete output;
}

// ---------------------------------------------------------------
// RevBasicMemCtrl
// ---------------------------------------------------------------
RevBasicMemCtrl::RevBasicMemCtrl(ComponentId_t id, Params& params)
  : RevMemCtrl(id,params), memIface(nullptr),stdMemHandlers(nullptr){

  stdMemHandlers = new RevBasicMemCtrl::RevStdMemHandlers(this,output);

  std::string ClockFreq = params.find<std::string>("clock", "1Ghz");

  memIface = loadUserSubComponent<Interfaces::StandardMem>(
    "memIface", ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
    getTimeConverter(ClockFreq), new StandardMem::Handler<SST::RevCPU::RevBasicMemCtrl>(
      this, &RevBasicMemCtrl::processMemEvent));


  registerClock( ClockFreq,
              new Clock::Handler<RevBasicMemCtrl>(this,&RevBasicMemCtrl::clockTick));
}

RevBasicMemCtrl::~RevBasicMemCtrl(){
  delete stdMemHandlers;
}

void RevBasicMemCtrl::processMemEvent(StandardMem::Request* ev){
}

void RevBasicMemCtrl::init(unsigned int phase){
  memIface->init(phase);
}

bool RevBasicMemCtrl::clockTick(Cycle_t cycle){
  return false;
}

// ---------------------------------------------------------------
// RevStdMemHandlers
// ---------------------------------------------------------------
RevBasicMemCtrl::RevStdMemHandlers::RevStdMemHandlers( RevBasicMemCtrl* Ctrl,
                                                       SST::Output* output)
  : Interfaces::StandardMem::RequestHandler(output), ctrl(Ctrl){
}

RevBasicMemCtrl::RevStdMemHandlers::~RevStdMemHandlers(){
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::ReadResp* ev){
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::WriteResp* ev){
}


// EOF
