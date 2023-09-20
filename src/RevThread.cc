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

using namespace SST::RevCPU;

RevThread::RevThread( uint32_t inputParentThreadID,
                      uint64_t inputStackPtr,
                      uint64_t inputFirstPC,
                      std::shared_ptr<MemSegment>& inputThreadMem,
                      RevFeature* inputFeature)
   : ParentThreadID(inputParentThreadID), StackPtr(inputStackPtr),  
     FirstPC(inputFirstPC), ThreadMem(inputThreadMem), Feature(inputFeature) {
  // Create the RegFile for this thread
  RegFile = RevRegFile(); 
  
  ThreadPtr = inputThreadMem->getTopAddr() - 1;
  // Set the stack pointer 
  RegFile.SetX(Feature, 2, StackPtr - 1024);

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
/// Used for duplicating register files (currently only in ECALL_clone)
// bool RevThread::DuplicateRegFile(const RevRegFile& regToDup){
//   RegFile = regToDup;
//   RegFile.RV32_SCAUSE = 0;
//   return true;
// }
