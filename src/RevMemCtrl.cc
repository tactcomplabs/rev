//
// _RevMemCtrl_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevMemCtrl.h"

using namespace SST;
using namespace RevCPU;

// ---------------------------------------------------------------
// RevMemOp
// ---------------------------------------------------------------
RevMemOp::RevMemOp(uint64_t Addr, uint64_t PAddr, uint32_t Size,
                   RevMemOp::MemOp Op, StandardMem::Request::flags_t flags )
  : Addr(Addr), PAddr(PAddr), Size(Size), Inv(false), Op(Op), CustomOpc(0),
    flags(flags), target(nullptr){
}

RevMemOp::RevMemOp(uint64_t Addr, uint64_t PAddr, uint32_t Size, void *target,
                   RevMemOp::MemOp Op, StandardMem::Request::flags_t flags )
  : Addr(Addr), PAddr(PAddr), Size(Size), Inv(false), Op(Op), CustomOpc(0),
    flags(flags), target(target){
}

RevMemOp::RevMemOp(uint64_t Addr, uint64_t PAddr, uint32_t Size,
                   char *buffer, RevMemOp::MemOp Op,
                   StandardMem::Request::flags_t flags )
  : Addr(Addr), PAddr(PAddr), Size(Size), Inv(false), Op(Op), CustomOpc(0),
    flags(flags), target(nullptr){
  for(unsigned i=0; i<(unsigned)(Size); i++ ){
    membuf.push_back((uint8_t)(buffer[i]));
  }
}

RevMemOp::RevMemOp(uint64_t Addr, uint64_t PAddr, uint32_t Size,
                   void *target, unsigned CustomOpc, RevMemOp::MemOp Op,
                   StandardMem::Request::flags_t flags )
  : Addr(Addr), PAddr(PAddr), Size(Size), Inv(false), Op(Op),
    CustomOpc(CustomOpc), flags(flags),
    target(target){
}

RevMemOp::RevMemOp(uint64_t Addr, uint64_t PAddr, uint32_t Size, char *buffer,
                   unsigned CustomOpc, RevMemOp::MemOp Op,
                   StandardMem::Request::flags_t flags )
  : Addr(Addr), PAddr(PAddr), Size(Size), Inv(false), Op(Op),
    CustomOpc(CustomOpc), flags(flags), target(nullptr){
  for(unsigned i=0; i<(unsigned)(Size); i++ ){
    membuf.push_back((uint8_t)(buffer[i]));
  }
}

RevMemOp::~RevMemOp(){
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
  : RevMemCtrl(id,params), memIface(nullptr), stdMemHandlers(nullptr),
    hasCache(false), lineSize(0),
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

  rqstQ.reserve(max_ops);

  memIface = loadUserSubComponent<Interfaces::StandardMem>(
    "memIface", ComponentInfo::SHARE_NONE,//*/ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
    getTimeConverter(ClockFreq), new StandardMem::Handler<SST::RevCPU::RevBasicMemCtrl>(
      this, &RevBasicMemCtrl::processMemEvent));

  if( !memIface ){
    output->fatal(CALL_INFO, -1, "Error : memory interface is null\n");
  }

  registerStats();

  registerClock( ClockFreq,
              new Clock::Handler<RevBasicMemCtrl>(this,&RevBasicMemCtrl::clockTick));
}

RevBasicMemCtrl::~RevBasicMemCtrl(){
  for( auto i=0; i<rqstQ.size(); i++ ){
    delete rqstQ[i];
  }
  rqstQ.clear();
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
                                       uint64_t PAddr,
                                       uint32_t Size,
                                       bool Inv,
                                       StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, RevMemOp::MemOp::MemOpFLUSH, flags);
  Op->setInv(Inv);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::FlushPending,1);
  return true;
}

bool RevBasicMemCtrl::sendREADRequest(uint64_t Addr,
                                      uint64_t PAddr,
                                      uint32_t Size,
                                      void *target,
                                      StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, target, RevMemOp::MemOp::MemOpREAD, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::ReadPending,1);
  return true;
}

bool RevBasicMemCtrl::sendWRITERequest(uint64_t Addr,
                                       uint64_t PAddr,
                                       uint32_t Size,
                                       char *buffer,
                                       StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, buffer, RevMemOp::MemOp::MemOpWRITE, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::WritePending,1);
  return true;
}

bool RevBasicMemCtrl::sendREADLOCKRequest(uint64_t Addr,
                                          uint64_t PAddr,
                                          uint32_t Size,
                                          void *target,
                                          StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, target, RevMemOp::MemOp::MemOpREADLOCK, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::ReadLockPending,1);
  return true;
}

bool RevBasicMemCtrl::sendWRITELOCKRequest(uint64_t Addr,
                                           uint64_t PAddr,
                                           uint32_t Size,
                                           char *buffer,
                                           StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, buffer, RevMemOp::MemOp::MemOpWRITEUNLOCK, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::WriteUnlockPending,1);
  return true;
}

bool RevBasicMemCtrl::sendLOADLINKRequest(uint64_t Addr,
                                          uint64_t PAddr,
                                          uint32_t Size,
                                          StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, RevMemOp::MemOp::MemOpLOADLINK, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::LoadLinkPending,1);
  return true;
}

bool RevBasicMemCtrl::sendSTORECONDRequest(uint64_t Addr,
                                           uint64_t PAddr,
                                           uint32_t Size,
                                           char *buffer,
                                           StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, buffer, RevMemOp::MemOp::MemOpSTORECOND, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::StoreCondPending,1);
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMREADRequest(uint64_t Addr,
                                            uint64_t PAddr,
                                            uint32_t Size,
                                            void *target,
                                            unsigned Opc,
                                            StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, target, Opc, RevMemOp::MemOp::MemOpCUSTOM, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::CustomPending,1);
  return true;
}

bool RevBasicMemCtrl::sendCUSTOMWRITERequest(uint64_t Addr,
                                             uint64_t PAddr,
                                             uint32_t Size,
                                             char *buffer,
                                             unsigned Opc,
                                             StandardMem::Request::flags_t flags){
  RevMemOp *Op = new RevMemOp(Addr, PAddr, Size, buffer, Opc, RevMemOp::MemOp::MemOpCUSTOM, flags);
  rqstQ.push_back(Op);
  recordStat(RevBasicMemCtrl::MemCtrlStats::CustomPending,1);
  return true;
}

bool RevBasicMemCtrl::sendFENCE(){
  RevMemOp *Op = new RevMemOp(0x00ull, 0x00ull, 0x00, RevMemOp::MemOp::MemOpFENCE, 0x00);
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
  if( !physMem )
    output->fatal(CALL_INFO, -1, "Error : not backing physical memory found\n" );
  memIface->init(phase);

  lineSize = memIface->getLineSize();
  if( lineSize > 0 ){
    output->verbose(CALL_INFO, 5, 0, "Detected cache layers; default line size=%d\n", lineSize);
    hasCache = true;
  }else{
    output->verbose(CALL_INFO, 5, 0, "No cache detected; disabling caching\n");
    hasCache = false;
  }
}

void RevBasicMemCtrl::setup(){
  memIface->setup();
}

void RevBasicMemCtrl::finish(){
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
    output->fatal(CALL_INFO, -1, "Error : unknown memory operation type\n");
    return false;
    break;
  }
  return false;
}

bool RevBasicMemCtrl::buildStandardMemRqst(RevMemOp *op){
  Interfaces::StandardMem::Request *rqst = nullptr;

  if( !op ){
    return false;
  }

#ifdef _REV_DEBUG_
  std::cout << "building mem request for addr=0x"
            << std::hex << op->getAddr() << std::dec << std::endl;
  if( op->getAddr() % 64 )
    std::cout << "WARNING: address is not cache aligned!" << std::endl;
#endif

  StandardMem::Request::flags_t TmpFlags;
  if( hasCache ){
    // grab the flags with all the caching attributes enabled
    TmpFlags = op->getStdFlags();
  }else{
    // disable the caching flags
    TmpFlags = op->getNoCacheFlags();
  }

  switch(op->getOp()){
  case RevMemOp::MemOp::MemOpREAD:
    rqst = new Interfaces::StandardMem::Read(op->getAddr(),
                                             (uint64_t)(op->getSize()),
                                             TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(ReadInFlight,1);
    num_read++;
    break;
  case RevMemOp::MemOp::MemOpWRITE:
    rqst = new Interfaces::StandardMem::Write(op->getAddr(),
                                              (uint64_t)(op->getSize()),
                                              op->getBuf(),
                                              TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(WriteInFlight,1);
    num_write++;
    break;
  case RevMemOp::MemOp::MemOpFLUSH:
    rqst = new Interfaces::StandardMem::FlushAddr(op->getAddr(),
                                                  (uint64_t)(op->getSize()),
                                                  op->getInv(),
                                                  (uint64_t)(op->getSize()),
                                                  TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(FlushInFlight,1);
    num_flush++;
    break;
  case RevMemOp::MemOp::MemOpREADLOCK:
    rqst = new Interfaces::StandardMem::ReadLock(op->getAddr(),
                                                 (uint64_t)(op->getSize()),
                                                 TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(ReadLockInFlight,1);
    num_readlock++;
    break;
  case RevMemOp::MemOp::MemOpWRITEUNLOCK:
    rqst = new Interfaces::StandardMem::WriteUnlock(op->getAddr(),
                                                    (uint64_t)(op->getSize()),
                                                    op->getBuf(),
                                                    false,
                                                    TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(WriteUnlockInFlight,1);
    num_writeunlock++;
    break;
  case RevMemOp::MemOp::MemOpLOADLINK:
    rqst = new Interfaces::StandardMem::LoadLink(op->getAddr(),
                                                 (uint64_t)(op->getSize()),
                                                 TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(LoadLinkInFlight,1);
    num_llsc++;
    break;
  case RevMemOp::MemOp::MemOpSTORECOND:
    rqst = new Interfaces::StandardMem::StoreConditional(op->getAddr(),
                                                        (uint64_t)(op->getSize()),
                                                        op->getBuf(),
                                                        TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(StoreCondInFlight,1);
    num_llsc++;
    break;
  case RevMemOp::MemOp::MemOpCUSTOM:
    // TODO: need more support for custom memory ops
    rqst = new Interfaces::StandardMem::CustomReq(nullptr, TmpFlags);
    requests.push_back(rqst->getID());
    outstanding[rqst->getID()] = op;
    memIface->send(rqst);
    recordStat(CustomInFlight,1);
    num_custom++;
    break;
  case RevMemOp::MemOp::MemOpFENCE:
    // we should never get here with a FENCE operation
    // the FENCE is handled locally and never dispatch on the memIface
  default:
    return false;
    break;
  }
  return true;
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
  for( unsigned i=0; i<rqstQ.size(); i++ ){
    RevMemOp *op = rqstQ[i];
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
        // no need to build a StandardMem request
        t_max_ops = max_ops;
        rqstQ.erase(rqstQ.begin()+i);
        num_fence+=1;
        delete op;
        return true;
      }

      // build a StandardMem request
      if( !buildStandardMemRqst(op) ){
        output->fatal(CALL_INFO, -1, "Error : failed to build memory request");
        return false;
      }

      // sent the request, remove it
      rqstQ.erase(rqstQ.begin()+i);

      return true;
    }
  }

  // if we reach this point, then we've attempted to
  // process all the potential requests.  none exist
  // that can be dispatched at this time.
  t_max_ops = max_ops;

#ifdef _REV_DEBUG_
  for( unsigned i=0; i<rqstQ.size(); i++ ){
    std::cout << "rqstQ[" << i << "] = " << rqstQ[i]->getOp() << " @ 0x"
              << std::hex << rqstQ[i]->getAddr() << std::dec
              << "; physAddr = 0x" << std::hex << rqstQ[i]->getPhysAddr() << std::dec << std::endl;
  }
#endif

  return true;
}

void RevBasicMemCtrl::handleFlagResp(RevMemOp *op){
   StandardMem::Request::flags_t flags = op->getFlags();

   if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_SEXT32)) ){
     uint32_t *target = (uint32_t *)(op->getTarget());
     SEXTI(*target,32);
   }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_SEXT64)) ){
     uint64_t *target = (uint64_t *)(op->getTarget());
     SEXTI(*target,64);
   }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_ZEXT32)) ){
     uint32_t *target = (uint32_t *)(op->getTarget());
     ZEXTI(*target,32);
   }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_ZEXT64)) ){
     uint64_t *target = (uint64_t *)(op->getTarget());
     ZEXTI(*target,64);
   }
}

void RevBasicMemCtrl::handleReadResp(StandardMem::ReadResp* ev){
  if( std::find(requests.begin(),requests.end(),ev->getID()) != requests.end() ){
    requests.erase(std::find(requests.begin(),requests.end(),ev->getID()));
    RevMemOp *op = outstanding[ev->getID()];
    if( !op )
      output->fatal(CALL_INFO, -1, "RevMemOp is null in handleReadResp\n" );
#ifdef _REV_DEBUG_
    std::cout << "handleReadResp : id=" << ev->getID() << " @Addr= 0x"
              << std::hex << op->getAddr() << std::dec << std::endl;
    for( unsigned i=0; i<(unsigned)(op->getSize()); i++ ){
      std::cout << "               : data[" << i << "] = " << (unsigned)(ev->data[i]) << std::endl;
    }
#endif
    uint8_t *target = (uint8_t *)(op->getTarget());
    //uint8_t *BaseMem = (uint8_t *)(op->getPhysAddr());
    for( unsigned i=0; i<(unsigned)(op->getSize()); i++ ){
      //*target = BaseMem[i];
      *target = ev->data[i];
      target++;
    }
    // determine if we need to sign/zero extend
    handleFlagResp(op);
    delete op;
    outstanding.erase(ev->getID());
    delete ev;
  }else{
    output->fatal(CALL_INFO, -1, "Error : found unknown ReadResp\n");
  }
  num_read--;
}

void RevBasicMemCtrl::handleWriteResp(StandardMem::WriteResp* ev){
  if( std::find(requests.begin(),requests.end(),ev->getID()) != requests.end() ){
    requests.erase(std::find(requests.begin(),requests.end(),ev->getID()));
    RevMemOp *op = outstanding[ev->getID()];
    if( !op )
      output->fatal(CALL_INFO, -1, "RevMemOp is null in handleWriteResp\n" );
#ifdef _REV_DEBUG_
    std::cout << "handleWriteResp : id=" << ev->getID() << " @Addr= 0x"
              << std::hex << op->getAddr() << std::dec << std::endl;
#endif
    //std::vector<uint8_t> buf = op->getBuf();
    //uint8_t *BaseMem = (uint8_t *)(op->getPhysAddr());
    //for( unsigned i=0; i<(unsigned)(op->getSize()); i++ ){
      //BaseMem[i] = buf[i];
    //}
    delete op;
    outstanding.erase(ev->getID());
    delete ev;
  }else{
    output->fatal(CALL_INFO, -1, "Error : found unknown WriteResp\n");
  }
  num_write--;
}

void RevBasicMemCtrl::handleFlushResp(StandardMem::FlushResp* ev){
  if( std::find(requests.begin(),requests.end(),ev->getID()) != requests.end() ){
    requests.erase(std::find(requests.begin(),requests.end(),ev->getID()));
    RevMemOp *op = outstanding[ev->getID()];
    if( op )
      delete op;
    outstanding.erase(ev->getID());
    delete ev;
  }else{
    output->fatal(CALL_INFO, -1, "Error : found unknown FlushResp\n");
  }
  num_flush--;
}

void RevBasicMemCtrl::handleCustomResp(StandardMem::CustomResp* ev){
  if( std::find(requests.begin(),requests.end(),ev->getID()) != requests.end() ){
    requests.erase(std::find(requests.begin(),requests.end(),ev->getID()));
    RevMemOp *op = outstanding[ev->getID()];
    if( op )
      delete op;
    outstanding.erase(ev->getID());
    delete ev;
  }else{
    output->fatal(CALL_INFO, -1, "Error : found unknown CustomResp\n");
  }
  num_custom--;
}

void RevBasicMemCtrl::handleInvResp(StandardMem::InvNotify* ev){
  if( std::find(requests.begin(),requests.end(),ev->getID()) != requests.end() ){
    requests.erase(std::find(requests.begin(),requests.end(),ev->getID()));
    RevMemOp *op = outstanding[ev->getID()];
    if( op )
      delete op;
    outstanding.erase(ev->getID());
    delete ev;
  }else{
    output->fatal(CALL_INFO, -1, "Error : found unknown InvResp\n");
  }
}

uint64_t RevBasicMemCtrl::getTotalRqsts(){
  return num_read + num_write + num_llsc +
         num_readlock + num_writeunlock + num_custom;
}

bool RevBasicMemCtrl::outstandingRqsts(){
  return (requests.size() > 0 );
}

bool RevBasicMemCtrl::clockTick(Cycle_t cycle){

  // check to see if the top request is a FENCE
  if( num_fence > 0 ){
    if( (num_read + num_write + num_llsc +
         num_readlock + num_writeunlock + num_custom) != 0 ){
      // waiting for the outstanding ops to clear
      recordStat(RevBasicMemCtrl::MemCtrlStats::FencePending,1);
      return false;
    }else{
      // clear the fence and continue processing
      num_fence--;
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
  : Interfaces::StandardMem::RequestHandler(output), Ctrl(Ctrl){
}

RevBasicMemCtrl::RevStdMemHandlers::~RevStdMemHandlers(){
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::ReadResp* ev){
  Ctrl->handleReadResp(ev);
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::WriteResp* ev){
  Ctrl->handleWriteResp(ev);
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::FlushResp* ev){
  Ctrl->handleFlushResp(ev);
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::CustomResp* ev){
  Ctrl->handleCustomResp(ev);
}

void RevBasicMemCtrl::RevStdMemHandlers::handle(StandardMem::InvNotify* ev){
  Ctrl->handleInvResp(ev);
}

// EOF
