//
// _RevThread_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#include <iostream>
#include "../include/RevThread.h"

using namespace SST::RevCPU;

RevThread::RevThread( uint32_t inputThreadID,
                      uint32_t inputParentThreadID,
                      uint64_t inputStackPtr,
                      uint64_t inputFirstPC,
                      std::shared_ptr<RevMem::MemSegment>& inputThreadMem,
                      RevFeature* inputFeature)
   : ThreadID(inputThreadID), ParentThreadID(inputParentThreadID), StackPtr(inputStackPtr),  
     FirstPC(inputFirstPC), ThreadMem(inputThreadMem), Feature(inputFeature) {
  // Create the RegFile for this thread
  RegFile = RevRegFile(); 
  
  // Set the stack pointer 
  RegFile.SetX(Feature, 2, StackPtr );

  // Set the thread pointer
  ThreadPtr = ThreadMem->getTopAddr();
  RegFile.SetX(Feature, 2, ThreadPtr);

  // Set the PC 
  RegFile.SetPC(Feature, FirstPC);
}

bool RevThread::AddChildThreadID(uint32_t tid){
  if( std::find(ChildrenThreadIDs.begin(), ChildrenThreadIDs.end(), tid) != ChildrenThreadIDs.end() ){
    ChildrenThreadIDs.push_back(tid);
    return true;
  }
  return false;
}

bool RevThread::RemoveChildThreadID(uint32_t tid){
  auto ChildToErase = std::find(ChildrenThreadIDs.begin(), ChildrenThreadIDs.end(), tid); 
  if (ChildToErase != ChildrenThreadIDs.end() ){
    ChildrenThreadIDs.erase(ChildToErase);
    return true;
  }
  return false;
}

// Add new file descriptor 
void RevThread::AddFD(int fd){
  fildes.push_back(fd);
}

// See if file descriptor exists/is owned by Ctx 
bool RevThread::FindFD(int fd){
  // Check if the fd is owned by the current ctx 
  auto it = std::find(fildes.begin(), fildes.end(), fd);
  if( it != fildes.end() ){
    return true;
  }
  return false;
}

// Remove file descriptor from Ctx (ie. rev_close)
bool RevThread::RemoveFD(int fd){
  // Check if the fd is owned by the current ctx 
  auto it = std::find(fildes.begin(), fildes.end(), fd);

  if( it != fildes.end() ){
    // fd found, return true 
    fildes.erase(it);
    return true;
  }
  // Not found, return false
  return false;  
}

// Get a string representation of the thread's state
const std::string RevThread::GetStateString() {
  switch (State){
    case ThreadState::START:
      return "START";
    case ThreadState::READY:
      return "READY";
    case ThreadState::RUNNING:
      return "RUNNING";
    case ThreadState::BLOCKED:
      return "BLOCKED";
    case ThreadState::DONE:
      return "DONE";
    default:
      return "UNKNOWN";
  }
}


// EOF
