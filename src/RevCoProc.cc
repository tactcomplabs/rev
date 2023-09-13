//
// _RevCoProc_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevCoProc.h"

using namespace SST;
using namespace RevCPU;


// ---------------------------------------------------------------
// RevCoProc
// ---------------------------------------------------------------
RevCoProc::RevCoProc(ComponentId_t id, Params& params)
  : SubComponent(id), output(nullptr) {

  uint32_t verbosity = params.find<uint32_t>("verbose");
  output = new SST::Output("[RevCoProc @t]: ", verbosity, 0, SST::Output::STDOUT);

}

RevCoProc::~RevCoProc(){
  delete output;
}


// ---------------------------------------------------------------
// RevSimpleCoProc
// ---------------------------------------------------------------
RevSimpleCoProc::RevSimpleCoProc(ComponentId_t id, Params& params)
  : RevCoProc(id, params), num_instRetired(0) {

  std::string ClockFreq = params.find<std::string>("clock", "1Ghz");

  registerStats();

  //This would be used ot register the clock with SST Core
  /*registerClock( ClockFreq,
              new Clock::Handler<RevSimpleCoProc>(this, &RevSimpleCoProc::ClockTick));
  output->output("Registering subcomponent RevSimpleCoProc with frequency=%s\n", ClockFreq.c_str());*/
}

RevSimpleCoProc::~RevSimpleCoProc(){

};

bool RevSimpleCoProc::IssueInst(RevFeature *F, RevRegFile *R, RevMem *M, uint32_t Inst){
    RevCoProcInst inst = RevCoProcInst(Inst, F, R, M);
    std::cout << "CoProc instruction issued: " << std::hex << Inst << std::dec << std::endl;
    InstQ.push(inst);
    return true;
}

void RevSimpleCoProc::registerStats(){
   num_instRetired = registerStatistic<uint64_t>("InstRetired");
}

bool RevSimpleCoProc::Reset(){
  InstQ = {};
  return true;
}

bool RevSimpleCoProc::ClockTick(SST::Cycle_t cycle){
  if(!InstQ.empty()){
    uint32_t inst = InstQ.front().Inst;
    num_instRetired->addData(1);
    InstQ.pop();
    std::cout << "CoProcessor to execute instruction: " << std::hex << inst << std::endl;
  }
  return true;
}
