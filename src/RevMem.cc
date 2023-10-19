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
#include "../include/RevRand.h"
#include <cstring>
#include <cmath>
#include <utility>
#include <memory>
#include <mutex>
#include <functional>

using namespace SST::RevCPU;
using MemSegment = RevMem::MemSegment;

RevMem::RevMem( uint64_t MemSize, RevOpts *Opts, RevMemCtrl *Ctrl, SST::Output *Output )
  : memSize(MemSize), opts(Opts), ctrl(Ctrl), output(Output) {
  // Note: this constructor assumes the use of the memHierarchy backend
  pageSize = 262144; //Page Size (in Bytes)
  addrShift = lg(pageSize);
  nextPage = 0;

  // We initialize StackTop to the size of memory minus 1024 bytes
  // This allocates 1024 bytes for program header information to contain
  // the ARGC and ARGV information
  stacktop = (_REVMEM_BASE_ + memSize) - 1024;

  // Add the 1024 bytes for the program header information
  AddMemSegAt(stacktop, 1024);

}

RevMem::RevMem( uint64_t MemSize, RevOpts *Opts, SST::Output *Output )
  : memSize(MemSize), opts(Opts), ctrl(nullptr), output(Output) {

  // allocate the backing memory, zeroing it
  physMem = new char [memSize]{};
  pageSize = 262144; //Page Size (in Bytes)
  addrShift = lg(pageSize);
  nextPage = 0;

  if( !physMem )
    output->fatal(CALL_INFO, -1, "Error: could not allocate backing memory\n");

  // We initialize StackTop to the size of memory minus 1024 bytes
  // This allocates 1024 bytes for program header information to contain
  // the ARGC and ARGV information
  stacktop = (_REVMEM_BASE_ + memSize) - 1024;
  AddMemSegAt(stacktop, 1024);
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
  uint64_t rval = RevRand(0, (uint32_t{1} << width) - 1);

  // find an address to fault
  unsigned NBytes = RevRand(0, memSize-8);
  uint64_t *Addr = (uint64_t *)(&physMem[0] + NBytes);

  // write the fault (read-modify-write)
  *Addr |= rval;
  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:MEM: Memory fault %u bits at address : 0x%" PRIxPTR "\n",
                  width, reinterpret_cast<uintptr_t>(Addr));
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
                    const MemReq& req,
                    StandardMem::Request::flags_t flags){
  for( auto it = LRSC.begin(); it != LRSC.end(); ++it ){
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
  LRSC.push_back(std::tuple<unsigned, uint64_t,
                 unsigned, uint64_t*>(Hart, Addr, (unsigned)(aq|(rl<<1)),
                                      reinterpret_cast<uint64_t *>(Target)));

  // now handle the memory operation
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);
  //check to see if we're about to walk off the page....
  // uint32_t adjPageNum = 0;
  // uint64_t adjPhysAddr = 0;
  // uint64_t endOfPage = (pageMap[pageNum].first << addrShift) + pageSize;
  char *BaseMem = &physMem[physAddr];
  char *DataMem = (char *)(Target);

  if( ctrl ){
    ctrl->sendREADLOCKRequest(Hart, Addr, (uint64_t)(BaseMem),
                              Len, Target, req, flags);
  }else{
    memcpy(DataMem, BaseMem, Len);
    // clear the hazard
    req.MarkLoadComplete(req);
  }

  return true;
}

bool RevMem::SCBase(unsigned Hart, uint64_t Addr, size_t Len,
                    void *Data, void *Target, uint8_t aq, uint8_t rl,
                    StandardMem::Request::flags_t flags){
  std::vector<std::tuple<unsigned, uint64_t, unsigned, uint64_t*>>::iterator it;

  for( it = LRSC.begin(); it != LRSC.end(); ++it ){
    if( (Hart == std::get<LRSC_HART>(*it)) &&
        (Addr == std::get<LRSC_ADDR>(*it)) ){
      // existing reservation; test to see if the value matches
      uint64_t *TmpTarget = std::get<LRSC_VAL>(*it);
      uint64_t *TmpData = static_cast<uint64_t *>(Data);

      if( Len == 32 ){
        uint32_t A = 0;
        uint32_t B = 0;
        for( size_t i = 0; i < Len; i++ ){
          A |= uint32_t(TmpTarget[i]) << i;
          B |= uint32_t(TmpData[i]) << i;
        }
        if( (A & B) == 0 ){
          static_cast<uint32_t *>(Target)[0] = 1;
          return false;
        }
      }else{
        uint64_t A = 0;
        uint64_t B = 0;
        for( size_t i = 0; i < Len; i++ ){
          A |= TmpTarget[i] << i;
          B |= TmpData[i] << i;
        }
        if( (A & B) == 0 ){
          static_cast<uint64_t *>(Target)[0] = 1;
          return false;
        }
      }

      // everything has passed so far,
      // write the value back to memory
      WriteMem(Hart, Addr, Len, Data, flags);

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
        output->fatal(CALL_INFO, -1, "Error: Page allocated multiple times\n");
      }
      AddToTLB(vAddr, physAddr);
    }
    else {
      /* vAddr not a valid address */


      // #ifdef _REV_DEBUG_
      for( auto Seg : MemSegs ){
        std::cout << *Seg << std::endl;
      }

      for( auto Seg : ThreadMemSegs ){
        std::cout << *Seg << std::endl;
      }

      output->fatal(CALL_INFO, 11,
                    "Segmentation Fault: Virtual address 0x%" PRIx64 " (PhysAddr = 0x%" PRIx64 ") was not found in any mem segments\n",
                    vAddr, physAddr);
    }
  }
  return physAddr;
}

// This function will change a decent amount in an upcoming PR
bool RevMem::isValidVirtAddr(const uint64_t vAddr){
  for(const auto& Seg : MemSegs ){
    if( Seg->contains(vAddr) ){
      return true;
    }
  }

  for( const auto& Seg : ThreadMemSegs ){
    if( Seg->contains(vAddr) ){
      return true;
    }
  }
  return false;
}


uint64_t RevMem::AddMemSegAt(const uint64_t& BaseAddr, const uint64_t& SegSize){
  MemSegs.emplace_back(std::make_shared<MemSegment>(BaseAddr, SegSize));
  return BaseAddr;
}

// Check if memory segment is already allocated (We are okay with overlap... for now per ZMAGIC but not duplicate segments)
// Currently only the loader calls this (Static Mem allocation does not have to worry about checking the FreeMemSegs
// vector because there will be no FreeMemSegs that contain addresses in the static segments)
//
// AllocMem is the only way that a user can allocate & deallocate memory
uint64_t RevMem::AddRoundedMemSeg(uint64_t BaseAddr, const uint64_t& SegSize, size_t RoundUpSize){
  size_t RoundedSegSize = 0;

  // Make sure we're not dividing by zero
  if( RoundUpSize == 0 ){
    output->fatal(CALL_INFO, -1, "Error: RoundUpSize must be greater than 0\n");
  }

  uint64_t Remainder = SegSize % RoundUpSize;
  // See if we need to round up at all
  if( Remainder == 0 ){
    RoundedSegSize = SegSize;
  } else {
    RoundedSegSize = SegSize + RoundUpSize - Remainder;
  }

  uint64_t NewSegTopAddr = BaseAddr + RoundedSegSize;
  bool Added = false;

  // Check if memory segment is already allocated
  for( auto Seg : MemSegs ){
    // If it contains the base address
    if( Seg->contains(BaseAddr) ){
      // If it doesn't contain the top address, we need to expand it
      if( !Seg->contains(NewSegTopAddr) ){
        size_t BytesToExpandBy = NewSegTopAddr - Seg->getTopAddr();
        Seg->setSize(Seg->getSize() + BytesToExpandBy);
      } else {
        // If it contains the top address, we don't need to do anything
        output->verbose(CALL_INFO, 10, 99,
                        "Warning: Memory segment already allocated that contains the requested rounded allocation at %" PRIx64 "of size %" PRIu64 " Bytes\n", BaseAddr, SegSize);
      }
      // Return the containing segments Base Address
      BaseAddr = Seg->getBaseAddr();
      Added = true;
      break;
    } // --- End (if contains BaseAddr)

    else if ( !Seg->contains(BaseAddr) && Seg->contains(NewSegTopAddr) ){
      // Existing segment only contains the top part of the new segment, expand downwards
      Seg->setBaseAddr(BaseAddr);
      size_t BytesToExpandBy = Seg->getBaseAddr() - BaseAddr;
      Seg->setSize(Seg->getSize() + BytesToExpandBy);
      Added = true;
      break;
    }

  }
  if( !Added ){
    // BaseAddr & RoundedTopAddr not a part of a segment
    // Add rounded segment
    MemSegs.emplace_back(std::make_shared<MemSegment>(BaseAddr, RoundedSegSize));
  }

  return BaseAddr;
}

std::shared_ptr<MemSegment> RevMem::AddThreadMem(){
  // Calculate the BaseAddr of the segment
  uint64_t BaseAddr = NextThreadMemAddr - ThreadMemSize;
  ThreadMemSegs.emplace_back(std::make_shared<MemSegment>(BaseAddr, ThreadMemSize));
  // Page boundary between
  NextThreadMemAddr = BaseAddr - pageSize - 1;
  return ThreadMemSegs.back();
}

void RevMem::SetTLSInfo(const uint64_t& BaseAddr, const uint64_t& Size){
  TLSBaseAddr = BaseAddr;
  TLSSize += Size;
  ThreadMemSize = _STACK_SIZE_ + TLSSize;
  return;
}


// AllocMem differs from AddMemSeg because it first searches the FreeMemSegs
// vector to see if there is a free segment that will fit the new data
// If there is not a free segment, it will allocate a new segment at the end of the heap
uint64_t RevMem::AllocMem(const uint64_t& SegSize){
  output->verbose(CALL_INFO, 10, 99, "Attempting to allocate %" PRIu64 " bytes on the heap\n", SegSize);

  uint64_t NewSegBaseAddr = 0;
  // Check if there is a free segment that can fit the new data
  for( size_t i=0; i < FreeMemSegs.size(); i++ ){
    auto FreeSeg = FreeMemSegs[i];
    // if the FreeSeg is bigger than the new data, we can shrink it so it starts
    // after the new segment (SegSize)
    uint64_t oldFreeSegSize = FreeSeg->getSize();
    if( oldFreeSegSize > SegSize ){
      // New data will start where the free segment started
      NewSegBaseAddr = FreeSeg->getBaseAddr();
      MemSegs.emplace_back(std::make_shared<MemSegment>(NewSegBaseAddr, SegSize));
      FreeSeg->setBaseAddr(FreeSeg->getBaseAddr() + SegSize);
      FreeSeg->setSize(oldFreeSegSize - SegSize);
      return NewSegBaseAddr;
    }
    // New data will fit exactly in the free segment
    // ie. remove from FreeMemSegs & add to MemSegs
    else if (oldFreeSegSize == SegSize ){
      // New data will start where the free segment started
      NewSegBaseAddr = FreeSeg->getBaseAddr();
      MemSegs.emplace_back(std::make_shared<MemSegment>(NewSegBaseAddr, SegSize));
      FreeMemSegs.erase(FreeMemSegs.begin()+i);
      return NewSegBaseAddr;
    }
    // FreeSeg not big enough to fit the new data
    else {
      continue;
    }
  }

  // If we still haven't allocated, expand the heap
  if( !NewSegBaseAddr ){
    NewSegBaseAddr = heapend;
  }
  MemSegs.emplace_back(std::make_shared<MemSegment>(NewSegBaseAddr, SegSize));

  ExpandHeap(SegSize);

  return NewSegBaseAddr;
}

// AllocMemAt differs from AddMemSegAt because it first searches the FreeMemSegs
// vector to see if there is a free segment that will fit the new data
// If its unable to allocate at the location requested it will error. This may change in the future.
uint64_t RevMem::AllocMemAt(const uint64_t& BaseAddr, const uint64_t& SegSize){
  int ret = 0;
  output->verbose(CALL_INFO, 10, 99, "Attempting to allocate %" PRIu64 " bytes on the heap", SegSize);

  // Check if this range exists in the FreeMemSegs vector
  for( unsigned i=0; i < FreeMemSegs.size(); i++ ){
    auto FreeSeg = FreeMemSegs[i];
    if( FreeSeg->contains(BaseAddr, SegSize) ){
      // Check if were allocating on a boundary of FreeSeg
      // if not, were allocating in the middle
      if( FreeSeg->getBaseAddr() != BaseAddr && FreeSeg->getTopAddr() != (BaseAddr + SegSize) ){
        // Before: |-------------------- FreeSeg --------------------|
        // After:  |--- FreeSeg ---|- AllocedSeg -|--- NewFreeSeg ---|

        size_t OldFreeSegTop = FreeSeg->getTopAddr();

        // Shrink FreeSeg so it's size goes up to the new AllocedSeg's BaseAddr
        FreeSeg->setSize(BaseAddr - FreeSeg->getBaseAddr());

        // Create New AllocedSeg; this is done later on before returning

        // Create New FreeSeg that fills the upper part of the old FreeSeg
        uint64_t NewFreeSegBaseAddr = BaseAddr + SegSize;
        size_t NewFreeSegSize = OldFreeSegTop - NewFreeSegBaseAddr;
        FreeMemSegs.emplace_back(std::make_shared<MemSegment>(NewFreeSegBaseAddr, NewFreeSegSize));
      }

      // If were allocating at the beginning of a FreeSeg (That doesn't take up the whole segment)
      else if( FreeSeg->getBaseAddr() == BaseAddr && FreeSeg->getTopAddr() != (BaseAddr + SegSize) ){
        // - Before: |--------------- FreeSeg --------------|
        // - After:  |---- AllocedSeg ----|---- FreeSeg ----|
        FreeSeg->setBaseAddr(BaseAddr + SegSize);
      }

      // If were allocating at the end of a FreeSeg (ie. TopAddr is last allocated address)
      else if( FreeSeg->getBaseAddr() != BaseAddr && FreeSeg->getTopAddr() == (BaseAddr + SegSize) ) {
        // - Before: |--------------- FreeSeg --------------|
        // - After:  |---- FreeSeg ----|---- AllocedSeg ----|
        FreeSeg->setSize(FreeSeg->getSize() - SegSize);
      }

      // Entire segment is being occupied
      else {
        // - Before: |-------- FreeSeg -------|
        // - After:  |------ AllocedSeg ------|
        FreeMemSegs.erase(FreeMemSegs.begin()+i);
      }
      // Segment was allocated so return the BaseAddr
      ret = BaseAddr;
      break;
    }
  }

  if (ret) { // Found a place
    // Check if any addresses in the segment are already
    for( auto Seg : MemSegs ){
      // Check if either the baseAddr or topAddr of the potential new segment exists inside of an already allocated segment
      if( Seg->contains(BaseAddr) || Seg->contains(BaseAddr + SegSize) ){
        output->fatal(CALL_INFO, 11,
                      "Error: Attempting to allocate memory at address 0x%lx of size 0x%lx which contains memory that is"
                      "already allocated in the segment with BaseAddr = 0x%lx and Size 0x%lx\n",
                      BaseAddr, SegSize, Seg->getBaseAddr(), Seg->getSize());
      } else {
        continue;
      }
    }
    MemSegs.emplace_back(std::make_shared<MemSegment>(BaseAddr, SegSize));
  }

  return ret;
}


bool RevMem::FenceMem(unsigned Hart){
  if( ctrl ){
    return ctrl->sendFENCE(Hart);
  }
  return true;  // base RevMem support does nothing here
}

bool RevMem::AMOMem(unsigned Hart, uint64_t Addr, size_t Len,
                    void *Data, void *Target,
                    const MemReq& req,
                    StandardMem::Request::flags_t flags){
#ifdef _REV_DEBUG_
  std::cout << "AMO of " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif

  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);
  char *BaseMem = &physMem[physAddr];

  if( ctrl ){
    // sending to the RevMemCtrl
    ctrl->sendAMORequest(Hart, Addr, (uint64_t)(BaseMem), Len,
                         static_cast<char *>(Data), Target, req, flags);
  }else{
    // process the request locally
    char *TmpD = new char [8]{};
    char *TmpT = new char [8]{};
    std::memset(TmpD, 0, 8);
    std::memcpy(TmpD, static_cast<char *>(Data), Len);
    std::memset(TmpT, 0, 8);

    ReadMem(Hart, Addr, Len, Target, req, flags);

    std::memcpy(TmpT, static_cast<char *>(Target), Len);
    if( Len == 4 ){
      ApplyAMO(flags, Target, *(uint32_t *)(TmpD));
    }else{
      ApplyAMO(flags, Target, *(uint64_t *)(TmpD));
    }

    WriteMem(Hart, Addr, Len, Target, flags);

    if( Len == 4 ){
      std::memcpy((uint32_t *)(Target), (uint32_t *)(TmpT), Len);
    }else{
      std::memcpy((uint64_t *)(Target), (uint64_t *)(TmpT), Len);
    }

    // clear the hazard
    req.MarkLoadComplete(req);
    delete[] TmpD;
    delete[] TmpT;
  }

  return true;
}

bool RevMem::WriteMem( unsigned Hart, uint64_t Addr, size_t Len, const void *Data,
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


bool RevMem::WriteMem( unsigned Hart, uint64_t Addr, size_t Len, const void *Data ){
#ifdef _REV_DEBUG_
  std::cout << "Writing " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif

  TRACE_MEM_WRITE(Addr, Len, Data);

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
                     const MemReq& req, StandardMem::Request::flags_t flags){
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
  char *DataMem = static_cast<char *>(Target);

  if((physAddr + Len) > endOfPage){
    uint32_t span = (physAddr + Len) - endOfPage;
    adjPageNum = ((Addr+Len)-span) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, ((Addr+Len)-span));
    if( ctrl ){
      ctrl->sendREADRequest(Hart, Addr, (uint64_t)(BaseMem), Len, Target, req, flags);
    }else{
      for( unsigned i=0; i< (Len-span); i++ ){
        DataMem[i] = BaseMem[i];
      }
    }
    BaseMem = &physMem[adjPhysAddr];
    if( ctrl ){
      unsigned Cur = (Len-span);
      ctrl->sendREADRequest(Hart, Addr, (uint64_t)(BaseMem), Len, ((char*)Target)+Cur, req, flags);
    }else{
      unsigned Cur = (Len-span);
      for( unsigned i=0; i< span; i++ ){
        DataMem[Cur] = BaseMem[i];
        Cur++;
      }
      // clear the hazard - if this was an AMO operation then we will clear outside of this function in AMOMem()
      if(MemOp::MemOpAMO != req.ReqType){
        req.MarkLoadComplete(req);
      }
    }
#ifdef _REV_DEBUG_
    std::cout << "Warning: Reading off end of page... " << std::endl;
#endif
  }else{
    if( ctrl ){
      ctrl->sendREADRequest(Hart, Addr, (uint64_t)(BaseMem), Len, Target, req, flags);
    }else{
      for( unsigned i=0; i<Len; i++ ){
        DataMem[i] = BaseMem[i];
      }
      // clear the hazard- if this was an AMO operation then we will clear outside of this function in AMOMem()
      if(MemOp::MemOpAMO != req.ReqType){
        req.MarkLoadComplete(req);
      }
      TRACE_MEM_READ(Addr, Len, DataMem);
    }
  }

  memStats.bytesRead += Len;
  return true;
}



// This function is used to remove/shrink a memory segment
// You *must* deallocate a chunk of memory that STARTS on a previously
// allocated baseAddr
//
// Said in another way... you can't deallocate:
// - Across multiple segments
// - In the middle of segments
//
// Three possible scenarios:
// 1. Deallocating the entire segment
// - |---------- AllocedSeg -----------|
// - |----------- FreeSeg -------------|
//
// 2. Deallocating a partial part of a segment
// - |------------- AllocedSeg --------------|
// - |---- NewFreeSeg ----|--- AllocedSeg ---|
// If this is the situation, we also need to check if the segment
// before (ie. baseAddr - 1) is also free and if so, find that
// segment and merge it with the new free segment
// - |--- FreeSeg ---|------------- AllocedSeg --------------|
// - |--- FreeSeg ---|---- NewFreeSeg ----|--- AllocedSeg ---|
// - |--- FreeSeg ------------------------|--- AllocedSeg ---|
//
// 3. Deallocating memory that hasn't been allocated
// - |---- FreeSeg ----| ==> SegFault :/
uint64_t RevMem::DeallocMem(uint64_t BaseAddr, uint64_t Size){
  output->verbose(CALL_INFO, 10, 99,
                  "Attempting to deallocate %lul bytes starting at BaseAddr = 0x%lx\n",
                  Size, BaseAddr);

  int ret = -1;
  // Search through allocated segments for the segment that begins on the baseAddr
  for( unsigned i=0; i<MemSegs.size(); i++ ){
    auto AllocedSeg = MemSegs[i];
    // We don't allow memory to be deallocated if it's not on a segment boundary
    if( AllocedSeg->getBaseAddr() != BaseAddr ){
      continue;
    } else {
      // Found the segment we're deallocating...

      // Make sure we're not trying to free beyond the segment boundaries
      if( Size > AllocedSeg->getSize() ){
        output->fatal(CALL_INFO, 11, "Dealloc Error: Cannot free beyond the segment bounds. Attempted to"
                      "free from 0x%lx to 0x%lx however the highest address in the segment is 0x%lx",
                      BaseAddr, BaseAddr+Size, AllocedSeg->getTopAddr());
      }
      // (2.) Check if we're only deallocating a part of a segment
      else if( Size < AllocedSeg->getSize() ){
        output->verbose(CALL_INFO, 10, 99, "  => partial deallocation detected\n");
        uint64_t oldAllocedSize = AllocedSeg->getSize();
        // Free data starts where alloced data used to
        // Before: |------------------- AllocedSeg ------------------------|
        // After:  |--- FreeSeg ---|------------- AllocedSeg --------------|
        // Alloced data now starts after the dealloced data
        AllocedSeg->setBaseAddr(BaseAddr + Size);
        AllocedSeg->setSize(oldAllocedSize - Size);
        ret = 0;
        break;
      } // --- End Partial Deallocation
      // We are deallocating the entire segment (1.)
      else {
        output->verbose(CALL_INFO, 10, 99, "  => entire deallocation\n");
        // Delete it from MemSegs
        MemSegs.erase(MemSegs.begin() + i);
        ret = 0;
        break;
      }
    }
  }

  // We found a matching segment to deallocate
  if (ret == 0) {
    // Check if the address before the baseAddr is also free
    // If so, we can merge the two segments
    // - Before: |--- FreeSeg ---|---- NewFreeSeg ----|--- AllocedSeg ---|
    // - After:  |--- FreeSeg ------------------------|--- AllocedSeg ---|
    bool hasMerged = false;
    for( auto FreeSeg : FreeMemSegs ){
      // Check if the address that precedes the baseAddr is free
      if( FreeSeg->contains(BaseAddr-1) ){
        // We can merge the two segments
        // by setting the Size of the FreeSeg to be the sum of the two
        // and NOT creating a new FreeMemSeg
        output->verbose(CALL_INFO, 10, 99, "  => merging with previous free segment\n");
        FreeSeg->setSize(FreeSeg->getSize() + Size);
        // Dealloc success, return 0
        hasMerged = true;
        break;
      }
    }
    if (!hasMerged) {
      output->verbose(CALL_INFO, 10, 99, "  => allocating new free segment\n");
      // If we get here, the address that precedes the newly freed data is not free
      // We need to create a new FreeMemSeg that starts at the baseAddr of the previously
      // allocated data and is `Size` bytes long
      // - Before: |--------------------|--- AllocedSeg ---|
      // - After:  |---- NewFreeSeg ----|--- AllocedSeg ---|
      FreeMemSegs.emplace_back(std::make_shared<MemSegment>(BaseAddr, Size));
    }
  }

  // here were not able to find the memory to deallocate
  return ret;
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
    // Mark heap as free
    FreeMemSegs.emplace_back(std::make_shared<MemSegment>(EndOfStaticData+1, maxHeapSize));

    heapend = EndOfStaticData + 1;
    heapstart = EndOfStaticData + 1;
  }
  return;
}

uint64_t RevMem::ExpandHeap(uint64_t Size){
  // We don't want multiple concurrent processes changing the heapend
  // at the same time (ie. two ThreadCtx calling brk)
  uint64_t NewHeapEnd = heapend + Size;

  // Check if we are out of heap space (ie. heapend >= bottom of stack)
  if( NewHeapEnd > maxHeapSize ){
    output->fatal(CALL_INFO, 7,  "Out Of Memory --- Attempted to expand heap to 0x%" PRIx64
                  " which goes beyond the maxHeapSize = 0x%x set in the python configuration. "
                  "If unset, this value will be equal to 1/4 of memSize.\n", NewHeapEnd, maxHeapSize);
  }
  // update the heapend
  heapend = NewHeapEnd;

  return heapend;
}

// EOF
