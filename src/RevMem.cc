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
                RevMemCtrl *Ctrl, SST::Output *Output)
  : physMem(nullptr), memSize(MemSize), opts(Opts), ctrl(Ctrl), output(Output),
    stacktop(0x00ull) {
  // Note: this constructor assumes the use of the memHierarchy backend
  pageSize = 262144; //Page Size (in Bytes)
  addrShift = int(log(pageSize) / log(2.0));
  nextPage = 0;

  //stacktop = _REVMEM_BASE_ + memSize;
  stacktop = (_REVMEM_BASE_ + memSize) - (1024*1024);

  memStats.bytesRead = 0;
  memStats.bytesWritten = 0;
  memStats.doublesRead = 0;
  memStats.doublesWritten = 0;
  memStats.floatsRead = 0;
  memStats.floatsWritten = 0;
  memStats.TLBHits = 0;
  memStats.TLBMisses = 0;

  /*
   * The first mem segment is the entirety of the memory space specified in the .py 
   * This is updated once RevLoader initializes and we know where the static
   * memory ends (ie. __BSS_END__) at which point we replace this first segment with 
   * a segment representing the static memory (0 -> __BSS_END__)
   */
  // AddMemSeg(0, memSize+1);
}

RevMem::RevMem( unsigned long MemSize, RevOpts *Opts, SST::Output *Output)
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

  //stacktop = _REVMEM_BASE_ + memSize;
  stacktop = (_REVMEM_BASE_ + memSize) - _STACK_SIZE_;

  memStats.bytesRead = 0;
  memStats.bytesWritten = 0;
  memStats.doublesRead = 0;
  memStats.doublesWritten = 0;
  memStats.floatsRead = 0;
  memStats.floatsWritten = 0;
  memStats.TLBHits = 0;
  memStats.TLBMisses = 0;

  // AddMemSeg(0, memSize);
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
                    bool *Hazard,
                    StandardMem::Request::flags_t flags){
  std::vector<std::tuple<unsigned,uint64_t,unsigned,uint64_t*>>::iterator it;

  for( it = LRSC.begin(); it != LRSC.end(); ++it ){
    if( (Hart == std::get<LRSC_HART>(*it)) &&
        (Addr == std::get<LRSC_ADDR>(*it)) ){
      // existing reservation; return w/ error
      uint32_t *Tmp = reinterpret_cast<uint32_t *>(Target);
      Tmp[0] = 0x01ul;
      return false;
    }else if( (Hart != std::get<LRSC_HART>(*it)) &&
              (Addr == std::get<LRSC_ADDR>(*it)) ){
      // existing reservation; return w/ error
      uint32_t *Tmp = reinterpret_cast<uint32_t *>(Target);
      Tmp[0] = 0x01ul;
      return false;
    }
  }

  // didn't find a colliding object; add it
  LRSC.push_back(std::tuple<unsigned,uint64_t,
                 unsigned,uint64_t*>(Hart,Addr,(unsigned)(aq|(rl<<1)),
                                     reinterpret_cast<uint64_t *>(Target)));

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
    *Hazard = true;
    ctrl->sendREADLOCKRequest(Hart, Addr, (uint64_t)(BaseMem),
                              Len, Target, Hazard, flags);
  }else{
    for( unsigned i=0; i<Len; i++ ){
      DataMem[i] = BaseMem[i];
    }
    // clear the hazard
    *Hazard = false;
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
      uint64_t *TmpData = reinterpret_cast<uint64_t *>(Data);

      if( Len == 32 ){
        uint32_t A = 0;
        uint32_t B = 0;
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
        uint64_t A = 0;
        uint64_t B = 0;
        for( unsigned i=0; i<Len; i++ ){
          A |= ((uint64_t)(TmpTarget[i]) << i);
          B |= ((uint64_t)(TmpData[i]) << i);
        }
        if( (A & B) == 0 ){
          uint64_t *Tmp = reinterpret_cast<uint64_t *>(Target);
          Tmp[0] = 0x1;
          return false;
        }
      }

      // everything has passed so far,
      // write the value back to memory
      WriteMem(Hart, Addr,Len,Data,flags);

      // write zeros to target
      for( unsigned i=0; i<Len; i++ ){
        uint64_t *Tmp = reinterpret_cast<uint64_t *>(Target);
        Tmp[i] = 0x0;
      }

      // erase the entry
      LRSC.erase(it);
      return true;
    }
  }

  // failed, write a nonzero value to target
  uint32_t *Tmp = reinterpret_cast<uint32_t *>(Target);
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
  /* Check if vAddr is in the TLB */
  uint64_t physAddr = SearchTLB(vAddr);

  /* If not in TLB, physAddr will equal _INVALID_ADDR_ */
  if( physAddr == _INVALID_ADDR_ ){
    /* Check if vAddr is a valid address before translating to physAddr */
    if( isValidVirtAddr(vAddr) ){
      if(pageMap.count(pageNum) == 0){
        // First touch of this page, mark it as in use
        pageMap[pageNum] = std::pair<uint32_t, bool>(nextPage, true);
        physAddr = (nextPage << addrShift) + ((pageSize - 1) & vAddr);
#ifdef _REV_DEBUG_
        std::cout << "First Touch for page:" << pageNum << " addrShift:" << addrShift << " vAddr: 0x" << std::hex << vAddr << " PhsyAddr: 0x" << physAddr << std::dec << " Next Page: " << nextPage << std::endl;
#endif
        nextPage++;
      }else if(pageMap.count(pageNum) == 1){
        //We've accessed this page before, just get the physical address 
        physAddr = (pageMap[pageNum].first << addrShift) + ((pageSize - 1) & vAddr);
#ifdef _REV_DEBUG_
        std::cout << "Access for page:" << pageNum << " addrShift:" << addrShift << " vAddr: 0x" << std::hex << vAddr << " PhsyAddr: 0x" << physAddr << std::dec << " Next Page: " << nextPage << std::endl;
#endif
      }else{
        output->fatal(CALL_INFO, -1, "Error: Page allocated multiple times");
      }
      AddToTLB(vAddr, physAddr);
    }
    else {
      /* vAddr not a valid address */


      #ifdef _REV_DEBUG_
      for( auto Seg : MemSegs ){
        std::cout << *Seg << std::endl;
      }
      #endif

      
      output->fatal(CALL_INFO, 11, 
                    "Segmentation Fault: Virtual address 0x%lx (PhysAddr = 0x%lx) was not found in any mem segments\n",
                    vAddr, physAddr);
    }
  }
  return physAddr;
}

bool RevMem::isValidVirtAddr(const uint64_t vAddr){
  // std::cout << "Checking validity of vAddr = 0x" << std::hex << vAddr << std::endl;
  for(const auto& MemSeg : MemSegs ){
    if( MemSeg->contains(vAddr) ){
      /* Found the segment containing the vAddr... if it's not free then were good... if it is segfault */
      return !(MemSeg->isFree());      
    } 
  }
  if( vAddr >= (stacktop - _STACK_SIZE_ ) ){
    if( vAddr < memSize ){
      return true;
    }
    else {
      // PrintMemBounds();
      return false;
    }
  }

  if( vAddr >= heapstart && vAddr <= heapend ){
    return true;
  }
  return false;
}


uint64_t RevMem::AddMemSeg(const uint64_t& BaseAddr, const uint64_t SegSize){
  // TODO: Check to make sure there's no overlap
  uint64_t TopAddr = BaseAddr + SegSize;

  // Check if memory segment is already allocated (We are okay with overlap... for now per ZMAGIC but not duplicate segments)
  for( auto Seg : MemSegs ){
    if( Seg->contains(BaseAddr, SegSize) ){
      if( Seg->isFree() ){
        // If the segment is free, we can shrink it to fit the new segment
        ShrinkMemSeg(Seg, SegSize);
      }
      else {
        // TODO: Eventually add more checks once permissions are implemented
        output->verbose(CALL_INFO, 10, 99, 
        "Warning: Memory segment already allocated at 0x%lx of size %lu Bytes\n", BaseAddr, SegSize);
        return BaseAddr;
      }
    }
  } 
}


uint64_t RevMem::AddMemSeg(const uint64_t& BaseAddr, const uint64_t SegSize, const bool roundUpToPage){
  // Calculate the number of pages needed to fit the segment
  uint64_t NumPages = SegSize / __PAGE_SIZE__;
  // std::cout << "Adding Memory Segment of size " << SegSize << " Bytes" << std::endl;
  if( SegSize % __PAGE_SIZE__ != 0 ){
    NumPages++;
  }

  uint64_t RoundedTopAddr = NumPages*__PAGE_SIZE__;

  // Check if memory segment is already allocated (We are okay with overlap... for now per ZMAGIC but not duplicate segments)
  for( auto Seg : MemSegs ){
    if( Seg->contains(BaseAddr, RoundedTopAddr) ){
      if( Seg->isFree() ){
        // If the segment is free, we can shrink it to fit the new segment
        ShrinkMemSeg(Seg, RoundedTopAddr);
      }
      else {
        // TODO: Eventually add more checks once permissions are implemented
        output->verbose(CALL_INFO, 10, 99, 
        "Warning: Memory segment already allocated at 0x%lx of size %lu Bytes\n", BaseAddr, SegSize);
        return BaseAddr;
      }
    }
  } 

  MemSegs.emplace_back(std::make_shared<MemSegment>(BaseAddr, RoundedTopAddr));
  return BaseAddr;
}



uint64_t RevMem::AllocMem(const uint64_t SegSize){
  output->verbose(CALL_INFO, 10, 99, "Attempting to allocating %lul bytes on the heap", SegSize);
  for( auto Seg : MemSegs ){
    // Check if we need to increase size of heap or if we can fit in existing segment
    if ((Seg->isFree()) && (Seg->getSize() >= SegSize)){
      // TODO: Eventually check write permissions based on current thread
      ShrinkMemSeg(Seg, SegSize);
    }
  }
  // If we get to this point there is nothing free so we have to try to expand heap 
  // TODO: Eventually this should use paging but that's not a priority at this point.
  uint64_t NewBaseAddr = heapend;
  MemSegs.emplace_back(std::make_shared<MemSegment>(NewBaseAddr, SegSize));

  ExpandHeap(SegSize);
  return NewBaseAddr;
}


bool RevMem::FenceMem(unsigned Hart){
  if( ctrl ){
    return ctrl->sendFENCE(Hart);
  }
  return true;  // base RevMem support does nothing here
}

bool RevMem::AMOMem(unsigned Hart, uint64_t Addr, size_t Len,
                    void *Data, void *Target,
                    bool *Hazard,
                    StandardMem::Request::flags_t flags){
#ifdef _REV_DEBUG_
  std::cout << "AMO of " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif

  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);
  //check to see if we're about to walk off the page....
  uint32_t adjPageNum = 0;
  uint64_t adjPhysAddr = 0;
  uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Target);

  // set the hazard
  *Hazard = true;

  if( ctrl ){
    // sending to the RevMemCtrl
    ctrl->sendAMORequest(Hart, Addr, (uint64_t)(BaseMem),
                              Len, reinterpret_cast<char *>(Data),
                              Target, Hazard, flags);
  }else{
    // process the request locally
    if( Len == 4 ){
      // 32bit amo
      int32_t *TmpTarget = reinterpret_cast<int32_t *>(Target);
      uint32_t *TmpTargetU = reinterpret_cast<uint32_t *>(Target);
      int32_t *TmpData = reinterpret_cast<int32_t *>(Data);
      uint32_t *TmpDataU = reinterpret_cast<uint32_t *>(Data);

      if(       ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOADD) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget += *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOXOR) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget ^= *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOAND) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget &= *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOOR) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget |= *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMIN) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget = std::min(*TmpTarget,*TmpData);
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMAX) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget = std::max(*TmpTarget,*TmpData);
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMINU) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTargetU),Hazard,flags);
        *TmpTargetU = std::min(*TmpTargetU,*TmpDataU);
        WriteMem(Hart,Addr,Len,(void *)(TmpTargetU));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMAXU) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTargetU),Hazard,flags);
        *TmpTargetU = std::max(*TmpTargetU,*TmpDataU);
        WriteMem(Hart,Addr,Len,(void *)(TmpTargetU));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOSWAP) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget = *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }

    }else{
      // 64bit amo
      int64_t *TmpTarget = reinterpret_cast<int64_t *>(Target);
      uint64_t *TmpTargetU = reinterpret_cast<uint64_t *>(Target);
      int64_t *TmpData = reinterpret_cast<int64_t *>(Data);
      uint64_t *TmpDataU = reinterpret_cast<uint64_t *>(Data);

      if(       ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOADD) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget += *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOXOR) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget ^= *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOAND) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget &= *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOOR) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget |= *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMIN) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget = std::min(*TmpTarget,*TmpData);
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMAX) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget = std::max(*TmpTarget,*TmpData);
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMINU) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTargetU),Hazard,flags);
        *TmpTargetU = std::min(*TmpTargetU,*TmpDataU);
        WriteMem(Hart,Addr,Len,(void *)(TmpTargetU));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOMAXU) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTargetU),Hazard,flags);
        *TmpTargetU = std::max(*TmpTargetU,*TmpDataU);
        WriteMem(Hart,Addr,Len,(void *)(TmpTargetU));
      }else if( ((uint32_t)(flags) & (uint32_t)(RevCPU::RevFlag::F_AMOSWAP) ) > 0 ){
        ReadMem(Hart,Addr,Len,(void *)(TmpTarget),Hazard,flags);
        *TmpTarget = *TmpData;
        WriteMem(Hart,Addr,Len,(void *)(TmpTarget));
      }
    }
    // clear the hazard
    *Hazard = false;
  }

  return true;
}

bool RevMem::WriteMem( unsigned Hart, uint64_t Addr, size_t Len, void *Data,
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
      ctrl->sendWRITERequest(Hart, Addr,
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
      ctrl->sendWRITERequest(Hart, Addr,
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
      ctrl->sendWRITERequest(Hart, Addr,
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


bool RevMem::WriteMem( unsigned Hart, uint64_t Addr, size_t Len, void *Data ){
#ifdef _REV_DEBUG_
  std::cout << "Writing " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif
  if (Tracer) 
    Tracer->memWrite(Addr,Len,Data);

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
      ctrl->sendWRITERequest(Hart, Addr,
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
      ctrl->sendWRITERequest(Hart, Addr,
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
      ctrl->sendWRITERequest(Hart, Addr,
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

bool RevMem::ReadMem(unsigned Hart, uint64_t Addr, size_t Len, void *Target,
                     bool *Hazard, StandardMem::Request::flags_t flags){
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

  // set the hazard
  *Hazard = true;

  if((physAddr + Len) > endOfPage){
    uint32_t span = (physAddr + Len) - endOfPage;
    adjPageNum = ((Addr+Len)-span) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, ((Addr+Len)-span));
    if( ctrl ){
      ctrl->sendREADRequest(Hart, Addr, (uint64_t)(BaseMem), Len, Target, Hazard, flags);
    }else{
      for( unsigned i=0; i< (Len-span); i++ ){
        DataMem[i] = BaseMem[i];
      }
    }
    BaseMem = &physMem[adjPhysAddr];
    if( ctrl ){
      unsigned Cur = (Len-span);
      ctrl->sendREADRequest(Hart, Addr, (uint64_t)(BaseMem), Len, ((char*)Target)+Cur, Hazard, flags);
    }else{
      unsigned Cur = (Len-span);
      for( unsigned i=0; i< span; i++ ){
        DataMem[Cur] = BaseMem[i];
        Cur++;
      }
      // clear the hazard
      *Hazard = false;
    }
#ifdef _REV_DEBUG_
    std::cout << "Warning: Reading off end of page... " << std::endl;
#endif
  }else{
    if( ctrl ){
      ctrl->sendREADRequest(Hart, Addr, (uint64_t)(BaseMem), Len, Target, Hazard, flags);
    }else{
      for( unsigned i=0; i<Len; i++ ){
        DataMem[i] = BaseMem[i];
      }
      // clear the hazard
      *Hazard = false;
      // trace the read
      if (Tracer) Tracer->memRead(Addr,Len,(void*) DataMem);      
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

void RevMem::WriteU8( unsigned Hart, uint64_t Addr, uint8_t Value ){
  uint8_t Tmp = Value;
  if( !WriteMem(Hart, Addr,1,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U8)");
}

void RevMem::WriteU16( unsigned Hart, uint64_t Addr, uint16_t Value ){
  uint16_t Tmp = Value;
  if( !WriteMem(Hart, Addr,2,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U16)");
}

void RevMem::WriteU32( unsigned Hart, uint64_t Addr, uint32_t Value ){
  uint32_t Tmp = Value;
  if( !WriteMem(Hart, Addr,4,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U32)");
}

void RevMem::WriteU64( unsigned Hart, uint64_t Addr, uint64_t Value ){
  uint64_t Tmp = Value;
  if( !WriteMem(Hart, Addr,8,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (U64)");
}

void RevMem::WriteFloat( unsigned Hart, uint64_t Addr, float Value ){
  uint32_t Tmp = 0x00;
  std::memcpy(&Tmp,&Value,sizeof(float));
  memStats.floatsWritten++;
  if( !WriteMem(Hart, Addr,4,(void *)(&Tmp)) )
    output->fatal(CALL_INFO, -1, "Error: could not write memory (FLOAT)");
}

void RevMem::WriteDouble( unsigned Hart, uint64_t Addr, double Value ){
  uint64_t Tmp = 0x00;
  std::memcpy(&Tmp,&Value,sizeof(double));
  memStats.doublesWritten++;
  if( !WriteMem(Hart, Addr,8,(void *)(&Tmp)) )
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

 
// This function is used to remove/shrink a memory segment
// You *must* deallocate a chunk of memory that STARTS on a previously
// allocated baseAddr 
//
// Said in another way... you can't deallocate:
// - Across multiple segments
// - In the middle of segments 
//
// When a segment is deallocated... the memory segment does not get removed
// instead... 
// - If BaseAddr + Size equals an entire segment... we simply mark it as free
// - If it is less than a whole segment, we create a new segment at MemSegs[i+1]
//   which encompasses the non-free space [Size+1 -> CurrSeg.TopAddr]
//   and then we mark the CurrSeg as free and shrink it's Size to Size
//   which will automatically update the TopAddr
uint64_t RevMem::DeallocMem(uint64_t BaseAddr, uint64_t Size){
  for( unsigned i=0; i<MemSegs.size(); i++ ){
    
    auto CurrSeg = MemSegs[i]; 
    /* We don't allow memory to be unallocated if it's not on a segment boundary */
    if( CurrSeg->getBaseAddr() != BaseAddr ){
      continue;
    } else {
      /* Make sure were not trying to free beyond the segment boundaries */
      if( Size > CurrSeg->getSize() ){
        output->fatal(CALL_INFO, 11, "Unalloc Error: Cannot free beyond the segment bounds. Attempted to"
                                     "free from 0x%lx to 0x%lx however the highest address in the segment is 0x%lx",
                                     BaseAddr, BaseAddr+Size, CurrSeg->getTopAddr());
        return false;
      } 
      
      /* Check if were unallocating a partial part of a segment */
      else if( Size < CurrSeg->getSize() ){
        /* Need to make a new segment that spans t */
        CurrSeg->setIsFree(true);
        
        /* Create a new segment that spans from Size+1 -> TopAddr */
        /* (Default free state is false in MemSegment constructor) */
        MemSegs.emplace(MemSegs.begin()+i+1, std::make_shared<MemSegment>(CurrSeg->getBaseAddr()+Size+1, CurrSeg->getTopAddr()));

        /* Shrink to size of free segment */
        CurrSeg->setSize(Size);

        /* Check if previous segment is free and combine */
        if( i > 1 ){
          auto PrevSeg = MemSegs.at(i-1);
          if( PrevSeg->isFree() ){
            if( PrevSeg->getTopAddr() == (CurrSeg->getBaseAddr() - 1) ){
              // std::cout << "Combining Memory Segments" << std::endl;
              // We need to do the following:
              // - Set the previous segments size to the combined size
              //   (setSize function automatically adjusts `TopAddr`)
              //   also because we previously updated the size of the 
              //   CurrSeg this should encompass the updated free size
              PrevSeg->setSize(PrevSeg->getSize() + CurrSeg->getSize());
              
              // - Remove CurrSeg
              MemSegs.erase(MemSegs.begin() + i);
            } 
            // PrevSeg was not contiguous with CurrSeg... while this shouldn't happen it's not breaking
            output->verbose(CALL_INFO, 2, 2,
                            "Previous segment (idx = %ul) has TopAddr = 0x%lx which is not contiguous with"
                            "CurrSeg (idx = %ul) which has baseAddr = 0x%lx ", i-1, PrevSeg->getTopAddr(), i, CurrSeg->getBaseAddr());
          }
          // PrevSeg was not free... don't combine
        }
        // There is either only 1 segment or we are inside the static memory section (segment 1)
        // so we don't need to worry about combining anything
      }

      else {
        // std::cout << "Unallocating entire segment" << std::endl;
        // std::cout << *CurrSeg << std::endl;
        CurrSeg->setIsFree(true);
        // std::cout << *CurrSeg << std::endl;
      }
    }
  }
  return true;
}


uint64_t RevMem::ShrinkMemSeg(std::shared_ptr<MemSegment> Seg, const uint64_t NewSegSize){
  /* Check if there will be leftover memory in the free segment */
  if( Seg->getSize() > NewSegSize ){
    /* Create new segment that encompasses the leftover space */
    AddMemSeg(NewSegSize+1, Seg->getTopAddr()-NewSegSize);
    /* Create the new segment that won't be free */
    MemSegs.emplace_back(std::make_shared<MemSegment>(Seg->getBaseAddr(), NewSegSize));
  } else {
    /* New segment same size as free block so all we have to do is set it to not free */
    // TODO: We will have to eventually update permissions based on thread ctx
    Seg->setIsFree(false);
  }
}

/// @brief This function is called from the loader to initialize the heap
/// @param EndOfStaticData: The address of the end of the static data section (ie. end of .bss section)
void RevMem::InitHeap(const uint64_t& EndOfStaticData){
  if( EndOfStaticData == 0x00ull ){
    // Program didn't contain .text, .data, or .bss sections
    output->fatal(CALL_INFO, 7, 
                  "The loader was unable"
                  "to find a .text section in your executable. This is a bug."
                  "EndOfStaticData = 0x%lx which is less than or equal to 0",
                  EndOfStaticData);
  } else {
    heapend = EndOfStaticData;
    heapstart = EndOfStaticData;
  }
  return;
}

uint64_t RevMem::ExpandHeap(uint64_t Size){
  /* 
   * We don't want multiple concurrent processes changing the heapend 
   * at the same time (ie. two ThreadCtx calling brk)
   */
  std::unique_lock<std::mutex> lock(heap_mtx);
  // std::cout << "HeapEnd = 0x" << heapend << std::endl;
  uint64_t NewHeapEnd = heapend + Size;
  
  /* Check if we are out of heap space (ie. heapend >= bottom of stack) */
  if( NewHeapEnd > maxHeapSize ){
    output->fatal(CALL_INFO, 7,  "Out Of Memory --- Attempted to expand heap to 0x%lx which goes beyond the maxHeapSize = 0x%x set in the python configuration. If unset, this value will be equal to 1/4 of memSize.",
                  NewHeapEnd, maxHeapSize);
  }
  heapend = NewHeapEnd;
  return heapend;
}

// EOF
