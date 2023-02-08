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
  : RevMemCtrl(id,params), memIface(nullptr),stdMemHandlers(nullptr),
    max_loads(64), max_stores(64), max_ops(2){

  stdMemHandlers = new RevBasicMemCtrl::RevStdMemHandlers(this,output);

  std::string ClockFreq = params.find<std::string>("clock", "1Ghz");

  max_loads = params.find<unsigned>("max_loads", 64);
  max_stores = params.find<unsigned>("max_stores", 64);
  max_ops = params.find<unsigned>("ops_per_cycle", 2);

  memIface = loadUserSubComponent<Interfaces::StandardMem>(
    "memIface", ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
    getTimeConverter(ClockFreq), new StandardMem::Handler<SST::RevCPU::RevBasicMemCtrl>(
      this, &RevBasicMemCtrl::processMemEvent));


  registerStats();

  registerClock( ClockFreq,
              new Clock::Handler<RevBasicMemCtrl>(this,&RevBasicMemCtrl::clockTick));
}

RevBasicMemCtrl::~RevBasicMemCtrl(){
  delete stdMemHandlers;
}

void RevBasicMemCtrl::registerStats(){
  stats.push_back(registerStatistic<uint64_t>("ReadInFlight"));
  stats.push_back(registerStatistic<uint64_t>("ReadPending"));
  stats.push_back(registerStatistic<uint64_t>("ReadBytes"));
  stats.push_back(registerStatistic<uint64_t>("WriteInFlight"));
  stats.push_back(registerStatistic<uint64_t>("WritePending"));
  stats.push_back(registerStatistic<uint64_t>("WriteBytes"));
  stats.push_back(registerStatistic<uint64_t>("FlushInFlight"));
  stats.push_back(registerStatistic<uint64_t>("FlushPending"));
  stats.push_back(registerStatistic<uint64_t>("ReadLockInFlight"));
  stats.push_back(registerStatistic<uint64_t>("ReadLockPending"));
  stats.push_back(registerStatistic<uint64_t>("ReadLockBytes"));
  stats.push_back(registerStatistic<uint64_t>("WriteUnlockInFlight"));
  stats.push_back(registerStatistic<uint64_t>("WriteUnlockPending"));
  stats.push_back(registerStatistic<uint64_t>("WriteUnlockBytes"));
  stats.push_back(registerStatistic<uint64_t>("LoadLinkInFlight"));
  stats.push_back(registerStatistic<uint64_t>("LoadLinkPending"));
  stats.push_back(registerStatistic<uint64_t>("StoreCondInFlight"));
  stats.push_back(registerStatistic<uint64_t>("StoreCondPending"));
}

void RevBasicMemCtrl::recordStat(RevBasicMemCtrl::MemCtrlStats Stat, uint64_t Data){
  if( Stat > RevBasicMemCtrl::MemCtrlStats::StoreCondPending){
    // do nothing
    return ;
  }
  stats[Stat]->addData(Data);
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
