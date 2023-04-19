#include "../include/RevThreadCtx.h"
#include <algorithm>
#include <iostream>

RevThreadCtx::RevThreadCtx(uint32_t pid, uint64_t pc, 
               uint32_t parentPID, ThreadState initialState, 
               uint64_t startAddr, uint64_t memSize,
                           RevRegFile regFile)
: PID(pid), PC(pc), ParentPID(parentPID), State(ThreadState::Ready),
  MemInfoStartAddr(startAddr), MemInfoSize(memSize), RegFile(regFile) {}


RevThreadCtx::~RevThreadCtx() {
}


bool RevThreadCtx::AddChildPID(uint32_t pid){
  if( std::find(ChildrenPIDs.begin(), ChildrenPIDs.end(), pid) != ChildrenPIDs.end() ){
    ChildrenPIDs.push_back(pid);
    return true;
  }
  else{
    std::cout << "Child process with id = " << pid << "already exists" << std::endl;
    return false;
  }
}


bool RevThreadCtx::RemoveChildPID(uint32_t pid){
  auto ChildToErase = std::find(ChildrenPIDs.begin(), ChildrenPIDs.end(), pid); 
  if (ChildToErase != ChildrenPIDs.end() ){
    ChildrenPIDs.erase(ChildToErase);
    return true;
  }
  else{
    std::cout << "Child process with id = " << pid << "doesn't exist" << std::endl;
    return false;
  }
}

