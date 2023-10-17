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
RevCoProc::RevCoProc(ComponentId_t id, Params& params, RevProc* parent)
  : SubComponent(id), output(nullptr), parent(parent) {

  uint32_t verbosity = params.find<uint32_t>("verbose");
  output = new SST::Output("[RevCoProc @t]: ", verbosity, 0, SST::Output::STDOUT);
}

RevCoProc::~RevCoProc(){
  delete output;
}

// ---------------------------------------------------------------
// RevSimpleCoProc
// ---------------------------------------------------------------
RevSimpleCoProc::RevSimpleCoProc(ComponentId_t id, Params& params, RevProc* parent)
  : RevCoProc(id, params, parent), num_instRetired(0) {

  std::string ClockFreq = params.find<std::string>("clock", "1Ghz");
  cycleCount = 0;

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
  //parent->ExternalDepSet(CreatePasskey(), F->GetHartToExec(), 7, false);
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
    //parent->ExternalDepClear(CreatePasskey(), InstQ.front().Feature->GetHartToExec(), 7, false);
    num_instRetired->addData(1);
    parent->ExternalStallHart(CreatePasskey(), 0);
    InstQ.pop();
    std::cout << "CoProcessor to execute instruction: " << std::hex << inst << std::endl;
    cycleCount = cycle;
  }
    if((cycle - cycleCount) > 500){
      parent->ExternalReleaseHart(CreatePasskey(), 0);
    }
  return true;
}

// ---------------------------------------------------------------
// SimpleUpDownCoProc
// ---------------------------------------------------------------
SimpleUpDownCoProc::SimpleUpDownCoProc(ComponentId_t id, Params& params, RevProc* parent)
  : RevCoProc(id, params, parent), num_instRetired(0) {

  std::string ClockFreq = params.find<std::string>("clock", "1Ghz");
  cycleCount = 0;

    udaccel = new basim::UDAccelerator(64,0,0);
    /* Initialize Accelerator  */
    udaccel->initSetup(0, "./addi_1.bin", 0);

  registerStats();

}

SimpleUpDownCoProc::~SimpleUpDownCoProc(){

};

bool SimpleUpDownCoProc::IssueInst(){
  std::cout << "UpDown instruction issued: " << std::endl;
   bool rtn = false;
    bool instIssued = false;
    if(!instIssued && udaccel->isIdle()){
        uint8_t numop = 2;
        basim::eventword_t ev0(0);
        ev0.setNumOperands(numop);
        basim::operands_t op0(numop);
        //basim::regval_t* data = new regval_t[numop];
        long unsigned int* data = new long unsigned int[numop];
        for(auto i = 0; i < numop; i++)
                data[i] = i+5;
        op0.setData(data);
        basim::eventoperands_t eops(&ev0, &op0);
        udaccel->pushEventOperands(eops, 0);
        udaccel->pushEventOperands(eops, 1);
        std::cout << "About to simulate..." << std::endl;
        udaccel->simulate(2);
        instIssued = true;
    }else{
        std::cout << "Another simulate..." << std::endl;
        udaccel->simulate(2);
    }
    if((instIssued == true) && udaccel->isIdle()){
         rtn = true;
        std::cout << "Done." << std::endl;
    }
    //while(!udaccel->isIdle())
  return true;
}

void SimpleUpDownCoProc::registerStats(){
  num_instRetired = registerStatistic<uint64_t>("InstRetired");
}

bool SimpleUpDownCoProc::Reset(){
  return true;
}

bool SimpleUpDownCoProc::ClockTick(SST::Cycle_t cycle){
  return true;
}
