#include "../include/RevThreadCtx.h"
#include <algorithm>
#include <iostream>

RevThreadCtx::RevThreadCtx(uint32_t pid, uint32_t parentPID, 
              ThreadState initialState, RevRegFile regFile,
              uint64_t memInfoStartAddr, uint64_t memInfoSize) // : State(initialState) {}
: PID(pid), ParentPID(parentPID), MemInfoStartAddr(memInfoStartAddr),
  MemInfoSize(memInfoSize),  State(ThreadState::Ready), RegFile(regFile){}


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

