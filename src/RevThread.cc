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

#include "../include/RevThread.h"
#include <algorithm>
#include <iostream>

RevThread::RevThread( uint32_t inputParentThreadID,
                      uint64_t inputStackPtr,
                      uint64_t inputFirstPC,
                      std::shared_ptr<MemSegment>& inputThreadMem)
   : ParentThreadID(inputParentThreadID), StackPtr(inputStackPtr),  FirstPC(inputFirstPC), ThreadMem(inputThreadMem) {
  // Create the RegFile for this thread
  RegFile = RevRegFile(); 
  
  ThreadPtr = inputThreadMem->getBaseAddr() + _STACK_SIZE_ + 1;
  // Set the stack pointer 
  RegFile.RV32[2] = (uint32_t)StackPtr - 1024;
  RegFile.RV64[2] = StackPtr - 1024;

  // Set the thread pointer
  ThreadPtr = ThreadMem->getTopAddr();
  RegFile.RV32[2] = (uint32_t)ThreadPtr;
  RegFile.RV64[4] = ThreadPtr;

  // Set the PC 
  RegFile.RV32_PC = (uint32_t)FirstPC;
  RegFile.RV64_PC = FirstPC;

  
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

/* Add new file descriptor */
void RevThread::AddFD(int fd){
  fildes.push_back(fd);
}
/* See if file descriptor exists/is owned by Ctx */
bool RevThread::FindFD(int fd){
  /* Check if the fd is owned by the current ctx */
  auto it = std::find(fildes.begin(), fildes.end(), fd);
  if( it != fildes.end() ){
    return true;
  }
  return false;
}

/* Remove file descriptor from Ctx (ie. rev_close) */
bool RevThread::RemoveFD(int fd){
  /* Check if the fd is owned by the current ctx */
  auto it = std::find(fildes.begin(), fildes.end(), fd);

  if( it != fildes.end() ){
    /* fd found, return true */
    fildes.erase(it);
    return true;
  }
  /* Not found, return false */
  return false;  
}

// void RevThread::WaitForThread(uint32_t ThreadToJoin){
//   if( WaitingToJoinTID ){
//     // this is a bug, one thread shouldn't be waiting on more than one other thread
//     // output->fatal(CALL_INFO, 0, "Thread %i attempted to wait for thread %i, however, it was already waiting on thread %i - This is a bug\n");
//     // TODO: Add output object to thread and add error handling
//   }
//   WaitingToJoinTID = ThreadToJoin;
//   return; 
// }

