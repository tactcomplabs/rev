//
// _RevMem_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevMem.h"
#include <math.h>
#include <mutex>

RevMem::RevMem( unsigned long MemSize, RevOpts *Opts,
                RevMemCtrl *Ctrl, SST::Output *Output )
  : physMem(nullptr), memSize(MemSize), opts(Opts), ctrl(Ctrl), output(Output),
    stacktop(0x00ull) {
  // Note: this constructor assumes the use of the memHierarchy backend
  pageSize = 262144; //Page Size (in Bytes)
  addrShift = int(log(pageSize) / log(2.0));
  nextPage = 0;

  stacktop = _REVMEM_BASE_ + memSize;

  memStats.bytesRead = 0;
  memStats.bytesWritten = 0;
  memStats.doublesRead = 0;
  memStats.doublesWritten = 0;
  memStats.floatsRead = 0;
  memStats.floatsWritten = 0;
  memStats.TLBHits = 0;
  memStats.TLBMisses = 0;
}

RevMem::RevMem( unsigned long MemSize, RevOpts *Opts, SST::Output *Output )
  : physMem(nullptr), memSize(MemSize), opts(Opts), ctrl(nullptr), output(Output),
    stacktop(0x00ull) {

  // allocate the backing memory
  physMem = new char [memSize];
  pageSize = 262144; //Page Size (in Bytes)
  addrShift = int(log(pageSize) / log(2.0));
  nextPage = 0;

  if( !physMem )
    output->fatal(CALL_INFO, -1, "Error: could not allocate backing memory\n");

  // zero the memory
  for( unsigned long i=0; i<memSize; i++ ){
    physMem[i] = 0;
  }

  stacktop = _REVMEM_BASE_ + memSize;

  memStats.bytesRead = 0;
  memStats.bytesWritten = 0;
  memStats.doublesRead = 0;
  memStats.doublesWritten = 0;
  memStats.floatsRead = 0;
  memStats.floatsWritten = 0;
  memStats.TLBHits = 0;
  memStats.TLBMisses = 0;
}

RevMem::~RevMem(){
  if( physMem )
    delete[] physMem;
}

bool RevMem::outstandingRqsts(){
  if( ctrl ){
    return ctrl->outstandingRqsts();
  }

  // RevMemCtrl is not enabled; no outstanding requests
  return false;
}

void RevMem::HandleMemFault(unsigned width){
  // build up the fault payload
  srand(time(NULL));
  uint64_t rval = rand() % (2^(width));

  // find an address to fault
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(0, memSize-8); // define the range
  unsigned NBytes = distr(gen);
  uint64_t *Addr = (uint64_t *)(&physMem[0] + NBytes);

  // write the fault (read-modify-write)
  *Addr |= rval;
  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:MEM: Memory fault %d bits at address : 0x%" PRIu64 "\n",
                 width, (uint64_t)(Addr));
}

bool RevMem::SetFuture(uint64_t Addr){
  FutureRes.push_back(Addr);
  std::sort( FutureRes.begin(), FutureRes.end() );
  FutureRes.erase( std::unique( FutureRes.begin(), FutureRes.end() ), FutureRes.end() );
  return true;
}

bool RevMem::RevokeFuture(uint64_t Addr){
  for( unsigned i=0; i<FutureRes.size(); i++ ){
    if( FutureRes[i] == Addr ){
      FutureRes.erase( FutureRes.begin() + i );
      return true;
    }
  }
  // nothing found
  return false;
}

bool RevMem::StatusFuture(uint64_t Addr){
  for( unsigned i=0; i<FutureRes.size(); i++ ){
    if( FutureRes[i] == Addr )
      return true;
  }
  return false;
}

bool RevMem::LRBase(unsigned Hart, uint64_t Addr, size_t Len,
                    void *Target, uint8_t aq, uint8_t rl,
                    StandardMem::Request::flags_t flags){
  std::vector<std::tuple<unsigned,uint64_t,unsigned,uint64_t*>>::iterator it;

  for( it = LRSC.begin(); it != LRSC.end(); ++it ){
    if( (Hart == std::get<LRSC_HART>(*it)) &&
        (Addr == std::get<LRSC_ADDR>(*it)) ){
      // existing reservation; return w/ error
      uint32_t *Tmp = (uint32_t *)(Target);
      Tmp[0] = 0x01ul;
      return false;
    }else if( (Hart != std::get<LRSC_HART>(*it)) &&
              (Addr == std::get<LRSC_ADDR>(*it)) ){
      // existing reservation; return w/ error
      uint32_t *Tmp = (uint32_t *)(Target);
      Tmp[0] = 0x01ul;
      return false;
    }
  }

  // didn't find a colliding object; add it
  LRSC.push_back(std::tuple<unsigned,uint64_t,
                 unsigned,uint64_t*>(Hart,Addr,(unsigned)(aq|(rl<<1)),
                                     (uint64_t *)(Target)));

  // now handle the memory operation
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);
  //check to see if we're about to walk off the page....
  uint32_t adjPageNum = 0;
  uint64_t adjPhysAddr = 0;
  uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Target);

  if( ctrl ){
    ctrl->sendREADLOCKRequest(Addr, (uint64_t)(BaseMem), Len, Target, flags);
  }else{
    for( unsigned i=0; i<Len; i++ ){
      DataMem[i] = BaseMem[i];
    }
  }

  return true;
}

bool RevMem::SCBase(unsigned Hart, uint64_t Addr, size_t Len,
                    void *Data, void *Target, uint8_t aq, uint8_t rl,
                    StandardMem::Request::flags_t flags){
  std::vector<std::tuple<unsigned,uint64_t,unsigned,uint64_t*>>::iterator it;

  for( it = LRSC.begin(); it != LRSC.end(); ++it ){
    if( (Hart == std::get<LRSC_HART>(*it)) &&
        (Addr == std::get<LRSC_ADDR>(*it)) ){
      // existing reservation; test to see if the value matches
      uint64_t *TmpTarget = std::get<LRSC_VAL>(*it);
      uint64_t *TmpData = (uint64_t *)(Data);

      if( Len == 32 ){
        uint32_t A;
        uint32_t B;
        for( unsigned i=0; i<Len; i++ ){
          A |= ((uint32_t)(TmpTarget[i]) << i);
          B |= ((uint32_t)(TmpData[i]) << i);
        }
        if( (A & B) == 0 ){
          uint32_t *Tmp = (uint32_t *)(Target);
          Tmp[0] = 0x1;
          return false;
        }
      }else{
        uint64_t A;
        uint64_t B;
        for( unsigned i=0; i<Len; i++ ){
          A |= ((uint64_t)(TmpTarget[i]) << i);
          B |= ((uint64_t)(TmpData[i]) << i);
        }
        if( (A & B) == 0 ){
          uint64_t *Tmp = (uint64_t *)(Target);
          Tmp[0] = 0x1;
          return false;
        }
      }

      // everything has passed so far,
      // write the value back to memory
      WriteMem(Addr,Len,Data,flags);

      // write zeros to target
      for( unsigned i=0; i<Len; i++ ){
        uint64_t *Tmp = (uint64_t *)(Target);
        Tmp[i] = 0x0;
      }

      // erase the entry
      LRSC.erase(it);
      return true;
    }
  }

  // failed, write a nonzero value to target
  uint32_t *Tmp = (uint32_t *)(Target);
  Tmp[0] = 0x1;

  return false;
}

unsigned RevMem::RandCost( unsigned Min, unsigned Max ){
  unsigned R = 0;

  srand(time(NULL));

  R = (unsigned)((rand() % Max) + Min);

  return R;
}

void RevMem::FlushTLB(){
  TLB.clear();
  LRUQueue.clear();
  return;
}

uint64_t RevMem::SearchTLB(uint64_t vAddr){
  auto it = TLB.find(vAddr);
  if (it == TLB.end()) {
      // TLB Miss :(
      memStats.TLBMisses++;
      return _INVALID_ADDR_;
  } else {
    memStats.TLBHits++;
    // Move the accessed vAddr to the front of the LRU list
    LRUQueue.erase(it->second.second);
    LRUQueue.push_front(vAddr);
    // Update the second of the pair in the tlbMap to point to the new location in LRU list
    it->second.second = LRUQueue.begin();
    // Return the physAddr
    return it->second.first;
  }
}

void RevMem::AddToTLB(uint64_t vAddr, uint64_t physAddr){
  auto it = TLB.find(vAddr);
  if (it != TLB.end()) {
    // If the vAddr is already present in the TLB,
    // then this is a page update, not a miss
    // Move the vAddr to the front of LRU list
    LRUQueue.erase(it->second.second);
    LRUQueue.push_front(vAddr);
    // Update the pair in the TLB
    it->second.first = physAddr;
    it->second.second = LRUQueue.begin();
  } else {
    // If cache is full, remove the least recently used
    // vAddr from both cache and LRU list
    if (LRUQueue.size() == tlbSize) {
      uint64_t LRUvAddr = LRUQueue.back();
      LRUQueue.pop_back();
      TLB.erase(LRUvAddr);
    }
    // Insert the vAddr and physAddr into the TLB and LRU list
    LRUQueue.push_front(vAddr);
    TLB.insert({vAddr, {physAddr, LRUQueue.begin()}});
  }
}

uint64_t RevMem::CalcPhysAddr(uint64_t pageNum, uint64_t vAddr){
  uint64_t physAddr = SearchTLB(vAddr);
  if( physAddr == _INVALID_ADDR_ ){
    if(pageMap.count(pageNum) == 0){
      // First touch of this page, mark it as in use
      pageMap[pageNum] = std::pair<uint32_t, bool>(nextPage, true);
      physAddr = (nextPage << addrShift) + ((pageSize - 1) & vAddr);
#ifdef _REV_DEBUG_
      std::cout << "First Touch for page:" << pageNum << " addrShift:"
                << addrShift << " vAddr: 0x" << std::hex << vAddr
                << " PhsyAddr: 0x" << physAddr << std::dec << " Next Page: "
                << nextPage << std::endl;
#endif
      nextPage++;
    }else if(pageMap.count(pageNum) == 1){
      //We've accessed this page before, just get the physical address
      physAddr = (pageMap[pageNum].first << addrShift) + ((pageSize - 1) & vAddr);
#ifdef _REV_DEBUG_
      std::cout << "Access for page:" << pageNum << " addrShift:"
                << addrShift << " vAddr: 0x" << std::hex << vAddr
                << " PhsyAddr: 0x" << physAddr << std::dec << " Next Page: "
                << nextPage << std::endl;
#endif
    }else{
      output->fatal(CALL_INFO, -1, "Error: Page allocated multiple times");
    }
    AddToTLB(vAddr, physAddr);
  }
  return physAddr;
}

bool RevMem::FenceMem(){
  if( ctrl ){
    return ctrl->sendFENCE();
  }
  return true;  // base RevMem support does nothing here
}

bool RevMem::WriteMem( uint64_t Addr, size_t Len, void *Data,
                       StandardMem::Request::flags_t flags){
#ifdef _REV_DEBUG_
  std::cout << "Writing " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif

  if(Addr == 0xDEADBEEF){
    std::cout << "Found special write. Val = " << std::hex << *(int*)(Data) << std::dec << std::endl;
  }
  RevokeFuture(Addr); // revoke the future if it is present; ignore the return
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);

  //check to see if we're about to walk off the page....
  uint32_t adjPageNum = 0;
  uint64_t adjPhysAddr = 0;
  uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Data);
  if((physAddr + Len) > endOfPage){
    uint32_t span = (physAddr + Len) - endOfPage;
    adjPageNum = ((Addr+Len)-span) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, ((Addr+Len)-span));
#ifdef _REV_DEBUG_
    std::cout << "Warning: Writing off end of page... " << std::endl;
#endif
    if( ctrl ){
      ctrl->sendWRITERequest(Addr,
                             (uint64_t)(BaseMem),
                             Len,
                             DataMem,
                             flags);
    }else{
      for( unsigned i=0; i< (Len-span); i++ ){
        BaseMem[i] = DataMem[i];
      }
    }
    BaseMem = &physMem[adjPhysAddr];
    if( ctrl ){
      // write the memory using RevMemCtrl
      unsigned Cur = (Len-span);
      ctrl->sendWRITERequest(Addr,
                             (uint64_t)(BaseMem),
                             Len,
                             &(DataMem[Cur]),
                             flags);
    }else{
      // write the memory using the internal RevMem model
      unsigned Cur = (Len-span);
      for( unsigned i=0; i< span; i++ ){
        BaseMem[i] = DataMem[Cur];
        Cur++;
      }
    }
  }else{
    if( ctrl ){
      // write the memory using RevMemCtrl
      ctrl->sendWRITERequest(Addr,
                             (uint64_t)(BaseMem),
                             Len,
                             DataMem,
                             flags);
    }else{
      // write the memory using the internal RevMem model
      for( unsigned i=0; i<Len; i++ ){
        BaseMem[i] = DataMem[i];
      }
    }
  }
  memStats.bytesWritten += Len;
  return true;
}


bool RevMem::WriteMem( uint64_t Addr, size_t Len, void *Data ){
#ifdef _REV_DEBUG_
  std::cout << "Writing " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif

  if(Addr == 0xDEADBEEF){
    std::cout << "Found special write. Val = " << std::hex << *(int*)(Data) << std::dec << std::endl;
  }
  RevokeFuture(Addr); // revoke the future if it is present; ignore the return
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);

  //check to see if we're about to walk off the page....
  uint32_t adjPageNum = 0;
  uint64_t adjPhysAddr = 0;
  uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Data);
  if((physAddr + Len) > endOfPage){
    uint32_t span = (physAddr + Len) - endOfPage;
    adjPageNum = ((Addr+Len)-span) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, ((Addr+Len)-span));

#ifdef _REV_DEBUG_
    std::cout << "ENDOFPAGE = " << std::hex << endOfPage << std::dec << std::endl;
    for( unsigned i=0; i<(Len-span); i++ ){
        std::cout << "WRITE TO: " << std::hex << (uint64_t)(&BaseMem[i]) << std::dec
                  << "; FROM LOGICAL PHYS=" << std::hex << physAddr + i << std::dec
                  << "; DATA=" << std::hex << (uint8_t)(BaseMem[i]) << std::dec
                  << "; VIRTUAL ADDR=" << std::hex << Addr+i << std::dec << std::endl;
    }

    std::cout << "TOTAL WRITE = " << Len << " Bytes" << std::endl;
    std::cout << "PHYS Writing " << Len-span << " Bytes Starting at 0x" << std::hex << physAddr << std::dec
              << "; translates to: " << std::hex << (uint64_t)(BaseMem) << std::dec << std::endl;
    std::cout << "ADJ PHYS Writing " << span << " Bytes Starting at 0x" << std::hex << adjPhysAddr << std::dec
              << "; translates to: " << std::hex << (uint64_t)(&physMem[adjPhysAddr]) << std::dec << std::endl;
    std::cout << "Warning: Writing off end of page... " << std::endl;
#endif
    if( ctrl ){
      ctrl->sendWRITERequest(Addr,
                             (uint64_t)(BaseMem),
                             Len,
                             DataMem,
                             0x00);
    }else{
      for( unsigned i=0; i< (Len-span); i++ ){
        BaseMem[i] = DataMem[i];
      }
    }
    BaseMem = &physMem[adjPhysAddr];
    if( ctrl ){
      // write the memory using RevMemCtrl
      unsigned Cur = (Len-span);
      ctrl->sendWRITERequest(Addr,
                             (uint64_t)(BaseMem),
                             Len,
                             &(DataMem[Cur]),
                             0x00);
    }else{
      // write the memory using the internal RevMem model
      unsigned Cur = (Len-span);
      for( unsigned i=0; i< span; i++ ){
        BaseMem[i] = DataMem[Cur];
#ifdef _REV_DEBUG_
        std::cout << "ADJ WRITE TO: " << std::hex << (uint64_t)(&BaseMem[i]) << std::dec
                  << "; FROM LOGICAL PHYS=" << std::hex << adjPhysAddr + i << std::dec
                  << "; DATA=" << std::hex << (uint8_t)(BaseMem[i]) << std::dec
                  << "; VIRTUAL ADDR=" << std::hex << Addr+Cur << std::dec << std::endl;
#endif
        Cur++;
      }
    }
  }else{
    if( ctrl ){
      // write the memory using RevMemCtrl
      ctrl->sendWRITERequest(Addr,
                             (uint64_t)(BaseMem),
                             Len,
                             DataMem,
                             0x00);
    }else{
      // write the memory using the internal RevMem model
      for( unsigned i=0; i<Len; i++ ){
        BaseMem[i] = DataMem[i];
      }
    }
  }
  memStats.bytesWritten += Len;
  return true;
}

bool RevMem::ReadMem( uint64_t Addr, size_t Len, void *Data ){
#ifdef _REV_DEBUG_
  std::cout << "OLD READMEM: Reading " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);

  //check to see if we're about to walk off the page....
  uint32_t adjPageNum = 0;
  uint64_t adjPhysAddr = 0;
  uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Data);
  if((physAddr + Len) > endOfPage){
    uint32_t span = (physAddr + Len) - endOfPage;
    adjPageNum = ((Addr+Len)-span) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, ((Addr+Len)-span));
    for( unsigned i=0; i< (Len-span); i++ ){
      DataMem[i] = BaseMem[i];
    }
    BaseMem = &physMem[adjPhysAddr];
    unsigned Cur = (Len-span);
    for( unsigned i=0; i< span; i++ ){
      DataMem[Cur] = BaseMem[i];
    }
#ifdef _REV_DEBUG_
    std::cout << "Warning: Reading off end of page... " << std::endl;
#endif
  }else{
    for( unsigned i=0; i<Len; i++ ){
      DataMem[i] = BaseMem[i];
    }
  }

  memStats.bytesRead += Len;
  return true;
}

bool RevMem::ReadMem(uint64_t Addr, size_t Len, void *Target,
                     StandardMem::Request::flags_t flags){
#ifdef _REV_DEBUG_
  std::cout << "NEW READMEM: Reading " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);
  //check to see if we're about to walk off the page....
  uint32_t adjPageNum = 0;
  uint64_t adjPhysAddr = 0;
  uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Target);
  if((physAddr + Len) > endOfPage){
    uint32_t span = (physAddr + Len) - endOfPage;
    adjPageNum = ((Addr+Len)-span) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, ((Addr+Len)-span));
    if( ctrl ){
      ctrl->sendREADRequest(Addr, (uint64_t)(BaseMem), Len, Target, flags);
    }else{
      for( unsigned i=0; i< (Len-span); i++ ){
        DataMem[i] = BaseMem[i];
      }
    }
    BaseMem = &physMem[adjPhysAddr];
    if( ctrl ){
      unsigned Cur = (Len-span);
      ctrl->sendREADRequest(Addr, (uint64_t)(BaseMem), Len, ((char*)Target)+Cur, flags);
    }else{
      unsigned Cur = (Len-span);
      for( unsigned i=0; i< span; i++ ){
        DataMem[Cur] = BaseMem[i];
        Cur++;
      }
    }
#ifdef _REV_DEBUG_
    std::cout << "Warning: Reading off end of page... " << std::endl;
#endif
  }else{
    if( ctrl ){
      ctrl->sendREADRequest(Addr, (uint64_t)(BaseMem), Len, Target, flags);
    }else{
      for( unsigned i=0; i<Len; i++ ){
        DataMem[i] = BaseMem[i];
      }
    }
  }

  memStats.bytesRead += Len;
  return true;
}

uint8_t RevMem::ReadU8( uint64_t Addr ){
  uint8_t Value;
  if( !ReadMem( Addr, 1, (void *)(&Value) ) )
    output->fatal(CALL_INFO, -1, "Error: could not read memory (U8)");
  return Value;
}

uint16_t RevMem::ReadU16( uint64_t Addr ){
  uint16_t Value;
  if( !ReadMem( Addr, 2, (void *)(&Value) ) )
    output->fatal(CALL_INFO, -1, "Error: could not read memory (U16)");
  return Value;
}

uint32_t RevMem::ReadU32( uint64_t Addr ){
  uint32_t Value;
  if( !ReadMem( Addr, 4, (void *)(&Value) ) )
    output->fatal(CALL_INFO, -1, "Error: could not read memory (U32)");
  return Value;
}

uint64_t RevMem::ReadU64( uint64_t Addr ){
  uint64_t Value;
  if( !ReadMem( Addr, 8, (void *)(&Value) ) )
    output->fatal(CALL_INFO, -1, "Error: could not read memory (U64)");
  return Value;
}

float RevMem::ReadFloat( uint64_t Addr ){
  float Value = 0.;
  uint32_t Tmp = 0x00;
  if( !ReadMem( Addr, 4, (void *)(&Tmp) ) )
    output->fatal(CALL_INFO, -1, "Error: could not read memory (FLOAT)");
  std::memcpy(&Value,&Tmp,sizeof(float));
  memStats.floatsRead++;
  return Value;
}

double RevMem::ReadDouble( uint64_t Addr ){
  double Value = 0.;
  uint64_t Tmp = 0x00;
  if( !ReadMem( Addr, 8, (void *)(&Tmp) ) )
    output->fatal(CALL_INFO, -1, "Error: could not read memory (DOUBLE)");
  std::memcpy(&Value,&Tmp,sizeof(double));
  memStats.doublesRead++;
  return Value;
}

void RevMem::WriteU8( uint64_t Addr, uint8_t Value ){
  uint8_t Tmp = Value;
  if( !WriteMem(Addr,1,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U8)");
}

void RevMem::WriteU16( uint64_t Addr, uint16_t Value ){
  uint16_t Tmp = Value;
  if( !WriteMem(Addr,2,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U16)");
}

void RevMem::WriteU32( uint64_t Addr, uint32_t Value ){
  uint32_t Tmp = Value;
  if( !WriteMem(Addr,4,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U32)");
}

void RevMem::WriteU64( uint64_t Addr, uint64_t Value ){
  uint64_t Tmp = Value;
  if( !WriteMem(Addr,8,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U64)");
}

void RevMem::WriteFloat( uint64_t Addr, float Value ){
  uint32_t Tmp = 0x00;
  std::memcpy(&Tmp,&Value,sizeof(float));
  memStats.floatsWritten++;
  if( !WriteMem(Addr,4,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (FLOAT)");
}

void RevMem::WriteDouble( uint64_t Addr, double Value ){
  uint64_t Tmp = 0x00;
  std::memcpy(&Tmp,&Value,sizeof(double));
  memStats.doublesWritten++;
  if( !WriteMem(Addr,8,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (DOUBLE)");
}

/*
* Func: GetNewThreadPID
* - This function is used to interact with the global 
*   PID counter inside of RevMem
* - When a new RevThreadCtx is created, it is assigned 
*   the value of PIDCount++
* - This ensures no collisions because all RevProcs access
*   the same RevMem instance
*/
uint32_t RevMem::GetNewThreadPID(){

  #ifdef _REV_DEBUG_
  std::cout << "RevMem: New PID being given: " << PIDCount+1 << std::endl; 
  #endif
  /*
  * NOTE: A mutex is acquired solely to prevent race conditions
  *       if multiple RevProc's create new Ctx objects at the 
  *       same time
  */
  std::unique_lock<std::mutex> lock(pid_mtx);
  PIDCount++;
  lock.unlock();
  return PIDCount;
}

// EOF
