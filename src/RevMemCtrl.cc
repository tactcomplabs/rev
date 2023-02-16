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
// RevMemOp
// ---------------------------------------------------------------
RevMemOp::RevMemOp(uint64_t Addr, uint32_t Size, RevMemOp::MemOp Op )
  : Addr(Addr), Size(Size), Op(Op), CustomOpc(0), membuf(nullptr), target(nullptr){
}

RevMemOp::RevMemOp(uint64_t Addr, uint32_t Size, void *target, RevMemOp::MemOp Op )
  : Addr(Addr), Size(Size), Op(Op), CustomOpc(0), membuf(nullptr), target(target){
}

RevMemOp::RevMemOp(uint64_t Addr, uint32_t Size,
                   char *buffer, RevMemOp::MemOp Op )
  : Addr(Addr), Size(Size), Op(Op), CustomOpc(0), membuf(nullptr), target(nullptr){
  membuf = new char[Size]();
  std::copy_n(buffer, Size, membuf);
}

RevMemOp::RevMemOp(uint64_t Addr, uint32_t Size,
                   void *target, unsigned CustomOpc, RevMemOp::MemOp Op )
  : Addr(Addr), Size(Size), Op(Op), CustomOpc(CustomOpc), membuf(nullptr), target(target){
}

RevMemOp::RevMemOp(uint64_t Addr, uint32_t Size, char *buffer,
                   unsigned CustomOpc, RevMemOp::MemOp Op )
  : Addr(Addr), Size(Size), Op(Op), CustomOpc(CustomOpc), membuf(nullptr), target(nullptr){
  membuf = new char[Size]();
  std::copy_n(buffer, Size, membuf);
}

RevMemOp::~RevMemOp(){
  if( membuf != nullptr )
    delete membuf;
}

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
    max_loads(64), max_stores(64), max_flush(64), max_llsc(64),
    max_readlock(64), max_writeunlock(64), max_custom(64), max_ops(2),
    num_read(0), num_write(0), num_flush(0), num_llsc(0), num_readlock(0),
    num_writeunlock(0), num_custom(0), num_fence(0){

  stdMemHandlers = new RevBasicMemCtrl::RevStdMemHandlers(this,output);

  std::string ClockFreq = params.find<std::string>("clock", "1Ghz");

  max_loads = params.find<unsigned>("max_loads", 64);
  max_stores = params.find<unsigned>("max_stores", 64);
  max_flush = params.find<unsigned>("max_flush", 64);
  max_llsc = params.find<unsigned>("max_llsc", 64);
  max_readlock = params.find<unsigned>("max_readlock", 64);
  max_writeunlock = params.find<unsigned>("max_writeunlock", 64);
  max_custom = params.find<unsigned>("max_custom", 64);
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
  stats.push_back(registerStatistic<uint64_t>("CustomInFlight"));
  stats.push_back(registerStatistic<uint64_t>("CustomPending"));
  stats.push_back(registerStatistic<uint64_t>("CustomBytes"));
  stats.push_back(registerStatistic<uint64_t>("FencePending"));
}

void RevBasicMemCtrl::recordStat(RevBasicMemCtrl::MemCtrlStats Stat,
                                 uint64_t Data){
  if( Stat > RevBasicMemCtrl::MemCtrlStats::FencePending){
    // do nothing
    return ;
  }
  stats[Stat]->addData(Data);
}

bool RevBasicMemCtrl::sendFLUSHRequest(uint64_t Addr,
                                       uint32_t Size){
  RevMemOp *Op = new RevMemOp(Addr, Size, RevMemOp::MemOp::MemOpFLUSH);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::FlushPending,1);
  return true;
}

bool RevBasicMemCtrl::sendREADRequest(uint64_t Addr,
                                      uint32_t Size,
                                      void *target){
  RevMemOp *Op = new RevMemOp(Addr, Size, target, RevMemOp::MemOp::MemOpREAD);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::ReadPending,1);
  return true;
}

bool RevBasicMemCtrl::sendWRITERequest(uint64_t Addr,
                                       uint32_t Size,
                                       char *buffer){
  RevMemOp *Op = new RevMemOp(Addr, Size, buffer, RevMemOp::MemOp::MemOpWRITE);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::WritePending,1);
  return true;
}

bool RevBasicMemCtrl::sendREADLOCKRequest(uint64_t Addr,
                                          uint32_t Size,
                                          void *target){
  RevMemOp *Op = new RevMemOp(Addr, Size, target, RevMemOp::MemOp::MemOpREADLOCK);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::ReadLockPending,1);
  return true;
}

bool RevBasicMemCtrl::sendWRITELOCKRequest(uint64_t Addr,
                                           uint32_t Size,
                                           char *buffer){
  RevMemOp *Op = new RevMemOp(Addr, Size, buffer, RevMemOp::MemOp::MemOpWRITEUNLOCK);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::WriteUnlockPending,1);
  return true;
}

bool RevBasicMemCtrl::sendLOADLINKRequest(uint64_t Addr,
                                          uint32_t Size){
  RevMemOp *Op = new RevMemOp(Addr, Size, RevMemOp::MemOp::MemOpLOADLINK);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::LoadLinkPending,1);
  return true;
}

bool RevBasicMemCtrl::sendSTORECONDRequest(uint64_t Addr,
                                           uint32_t Size,
                                           char *buffer){
  RevMemOp *Op = new RevMemOp(Addr, Size, buffer, RevMemOp::MemOp::MemOpSTORECOND);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::StoreCondPending,1);
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMREADRequest(uint64_t Addr,
                                            uint32_t Size,
                                            void *target,
                                            unsigned Opc){
  RevMemOp *Op = new RevMemOp(Addr, Size, target, Opc, RevMemOp::MemOp::MemOpCUSTOM);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::CustomPending,1);
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMWRITERequest(uint64_t Addr,
                                             uint32_t Size,
                                             char *buffer,
                                             unsigned Opc){
  RevMemOp *Op = new RevMemOp(Addr, Size, buffer, Opc, RevMemOp::MemOp::MemOpCUSTOM);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::CustomPending,1);
  return true;
}

bool RevBasicMemCtrl::sendFENCE(){
  RevMemOp *Op = new RevMemOp(0x00ull, 0x00, RevMemOp::MemOp::MemOpFENCE);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::FencePending,1);
  return true;
}

void RevBasicMemCtrl::processMemEvent(StandardMem::Request* ev){
  output->verbose(CALL_INFO, 15, 0, "Received memory request event\n");
  if( ev == nullptr ){
    output->fatal(CALL_INFO, -1, "Error : Received null memory event\n");
  }
  ev->handle(stdMemHandlers);
}

void RevBasicMemCtrl::init(unsigned int phase){
  memIface->init(phase);
}

bool RevBasicMemCtrl::isMemOpAvail(RevMemOp *Op,
                                   unsigned &t_max_loads,
                                   unsigned &t_max_stores,
                                   unsigned &t_max_flush,
                                   unsigned &t_max_llsc,
                                   unsigned &t_max_readlock,
                                   unsigned &t_max_writeunlock,
                                   unsigned &t_max_custom){
  switch(Op->getOp()){
  case RevMemOp::MemOp::MemOpREAD:
    if( t_max_loads < max_loads ){
      t_max_loads++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpWRITE:
    if( t_max_stores < max_stores ){
      t_max_stores++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpFLUSH:
    if( t_max_flush < max_flush ){
      t_max_flush++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpREADLOCK:
    if( t_max_readlock < max_readlock ){
      t_max_readlock++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpWRITEUNLOCK:
    if( t_max_writeunlock < max_writeunlock ){
      t_max_writeunlock++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpLOADLINK:
    if( t_max_llsc < max_llsc ){
      t_max_llsc++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpSTORECOND:
    if( t_max_llsc < max_llsc ){
      t_max_llsc++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpCUSTOM:
    if( t_max_custom < max_custom ){
      t_max_custom++;
      return true;
    }
    return false;
    break;
  case RevMemOp::MemOp::MemOpFENCE:
    return true;
    break;
  default:
    return false;
    break;
  }
  return false;
}

bool RevBasicMemCtrl::processNextRqst(unsigned &t_max_loads,
                                      unsigned &t_max_stores,
                                      unsigned &t_max_flush,
                                      unsigned &t_max_llsc,
                                      unsigned &t_max_readlock,
                                      unsigned &t_max_writeunlock,
                                      unsigned &t_max_custom,
                                      unsigned &t_max_ops){
  if( rqstQ.size() == 0 ){
    // nothing to do, saturate and exit this cycle
    t_max_ops = max_ops;
    return true;
  }

  // retrieve the next candidate memory operation
  for (auto &op : rqstQ) {
    if( isMemOpAvail(op,
                     t_max_loads,
                     t_max_stores,
                     t_max_flush,
                     t_max_llsc,
                     t_max_readlock,
                     t_max_writeunlock,
                     t_max_custom) ){
      // op is good to execute, build a StandardMem packet
      t_max_ops++;
      if( op->getOp() == RevMemOp::MemOp::MemOpFENCE ){
        // time to fence!
        // saturate and exit this cycle
        t_max_ops = max_ops;
      }
      return true;
    }
  }

  return true;
}

bool RevBasicMemCtrl::clockTick(Cycle_t cycle){

  // check to see if the top request is a FENCE
  if( rqstQ.front()->getOp() == RevMemOp::MemOp::MemOpFENCE ){
    if( (num_read + num_write + num_llsc +
         num_readlock + num_writeunlock + num_custom) != 0 ){
      // waiting for the outstanding ops to clear
      recordStat(RevBasicMemCtrl::MemCtrlStats::FencePending,1);
      return false;
    }else{
      // clear the fence and continue processing
      RevMemOp *TmpOp = rqstQ.front();
      rqstQ.pop_front();
      num_fence--;
      delete TmpOp;
    }
  }

  // process the memory queue
  bool done = false;
  unsigned t_max_ops = 0;
  unsigned t_max_loads = 0;
  unsigned t_max_stores = 0;
  unsigned t_max_flush = 0;
  unsigned t_max_llsc = 0;
  unsigned t_max_readlock = 0;
  unsigned t_max_writeunlock = 0;
  unsigned t_max_custom = 0;
  while( !done ){
    if( !processNextRqst(t_max_loads, t_max_stores, t_max_flush,
                         t_max_llsc, t_max_readlock, t_max_writeunlock,
                         t_max_custom, t_max_ops) ){
      // error occurred
      output->fatal(CALL_INFO, -1, "Error : failed to process next memory request");
    }

    if( t_max_ops == max_ops ){
      done = true;
    }
  }

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
