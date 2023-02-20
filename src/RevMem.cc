//
// _RevMem_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevMem.h"

RevMem::RevMem( unsigned long MemSize, RevOpts *Opts, SST::Output *Output )
  : memSize(MemSize), opts(Opts), output(Output), mem(nullptr), stacktop(0x00ull) {

  // allocate the backing memory
  mem = new char [memSize];
  if( !mem )
    output->fatal(CALL_INFO, -1, "Error: could not allocate backing memory");

  // zero the memory
  for( unsigned long i=0; i<memSize; i++ ){
    mem[i] = 0;
  }

  stacktop = _REVMEM_BASE_ + memSize;
}

RevMem::~RevMem(){
  delete[] mem;
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

bool RevMem::LR(unsigned Hart, uint64_t Addr){
  std::pair<unsigned,uint64_t> Entry = std::make_pair(Hart,Addr);
  LRSC.push_back(Entry);
  return true;
}

bool RevMem::SC(unsigned Hart, uint64_t Addr){
  // search the LRSC vector for the entry pair
  std::pair<unsigned,uint64_t> Entry = std::make_pair(Hart,Addr);
  std::vector<std::pair<unsigned,uint64_t>>::iterator it;

  for( it = LRSC.begin(); it != LRSC.end(); ++it ){
    if( (Hart == std::get<0>(*it)) &&
        (Addr == std::get<1>(*it)) ){
      LRSC.erase(it);
      return true;
    }
  }

  return false;
}

unsigned RevMem::RandCost( unsigned Min, unsigned Max ){
  unsigned R = 0;

  srand(time(NULL));

  R = (unsigned)((rand() % Max) + Min);

  return R;
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
    adjPageNum = (physAddr + Len) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, (physAddr + Len));
    uint32_t span = (physAddr + Len) - endOfPage;
    std::cout << "Warning: Writing off end of page... " << std::endl;
    for( unsigned i=0; i< (Len-span); i++ ){
      BaseMem[i] = DataMem[i];
    }
    BaseMem = &physMem[adjPhysAddr]; 
    for( unsigned i=0; i< span; i++ ){
      BaseMem[i] = DataMem[i];
    }
  }else{
    for( unsigned i=0; i<Len; i++ ){
      BaseMem[i] = DataMem[i];
    }
  }
  memStats.bytesWritten += Len;
  return true;
}

bool RevMem::ReadMem( uint64_t Addr, size_t Len, void *Data ){
#ifdef _REV_DEBUG_
  std::cout << "Reading " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
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
    adjPageNum = (physAddr + Len) >> addrShift;
    adjPhysAddr = CalcPhysAddr(adjPageNum, (physAddr + Len));
    uint32_t span = (physAddr + Len) - endOfPage;
    for( unsigned i=0; i< (Len-span); i++ ){
      DataMem[i] = BaseMem[i];
    }
    BaseMem = &physMem[adjPhysAddr]; 
    for( unsigned i=0; i< span; i++ ){
      DataMem[i] = BaseMem[i];
    }
#ifdef _REV_DEBUG_
    std::cout << "Warning: Reading off end of page... " << std::endl;
#endif
  }else{
    for( unsigned i=0; i<Len; i++ ){
      DataMem[i] = BaseMem[i];
    }
  }
  return true;
}

bool RevMem::FindStringTerminal( uint64_t Addr, size_t & Len ) {
#ifdef _REV_DEBUG_
  std::cout << "Reading " << Len << " Bytes Starting at 0x" << std::hex << Addr << std::dec << std::endl;
#endif
  uint64_t pageNum = Addr >> addrShift;
  uint64_t physAddr = CalcPhysAddr(pageNum, Addr);

  const static char nullTerminalCharacter = '\0';
  const auto pageMapEnd = std::end(pageMap);

  auto pageMapItr = pageMap[pageNum];

  while(pageMapItr != pageMapEnd) {
     //check to see if we're about to walk off the page....
     uint32_t adjPageNum = 0;
     uint64_t adjPhysAddr = 0;
     uint64_t endOfPage = (pageMapItr->first << addrShift) + pageSize;

     char *BaseMem = &physMem[physAddr]; 
     char *DataMem = (char *)(Data);
     if(physAddr > endOfPage){
       adjPageNum = physAddr >> addrShift;
       adjPhysAddr = CalcPhysAddr(adjPageNum, physAddr);
       uint32_t span = physAddr - endOfPage;
       for( unsigned i=0; i< span; i++ ){
         if(BaseMem[i] == nullTerminalCharacter) { Len = i; return true; }
     }
     BaseMem = &physMem[adjPhysAddr]; 
     for( unsigned i=0; i< span; i++ ){
        if(BaseMem[i] == nullTerminalCharacter) { Len = i; return true; }
     }
#ifdef _REV_DEBUG_
     std::cout << "Warning: Reading off end of page... " << std::endl;
#endif
    }else{
      for( unsigned i=0; i<std::numeric_limit<std::size_t>::max; i++ ){
        if(BaseMem[i] == nullTerminalCharacter) { Len = i; return true; }
      }
    }

    pageMapItr = pageMap[endOfPage+1];
  }
  return false;
}
