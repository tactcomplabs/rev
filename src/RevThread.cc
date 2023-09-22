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
                      std::shared_ptr<MemSegment>& inputThreadMem,
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
std::string RevThread::GetStateString() {
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

// Override the << operator for printing the thread
std::ostream& operator<<(std::ostream& os, RevThread& thread) {
  os << "\n";
  auto RegFile = thread.GetRegFile();
  RevFeature* Feature = thread.GetFeature();

  // Calculate total width of the table
  int tableWidth = 6 /*Reg*/ + 7 /*Alias*/ + 16 /*Value*/ + 23 /*Info*/ + 9 /*Separators*/;

  // Print a top border
  os << "|" << std::string(tableWidth-1, '=') << "|" << '\n';
  
  // Print Thread ID
  os << "| Thread " << thread.GetThreadID() << std::setw(6) <<  std::string(tableWidth-10, ' ') << "|\n";

  // Print the middle border
  os << "|" << std::string(tableWidth-1, '-') << "|" << '\n';

  // Print a nice header
  os << " ==> State: " << thread.GetStateString() << "\n";
  os << " ==> ParentTID: " << thread.GetParentThreadID() << "\n";
  os << " ==> Blocked by TID: " ;
  if (thread.GetWaitingToJoinTID() != __INVALID_TID__) {
    os << thread.GetWaitingToJoinTID();
  } else {
    os << "N/A";
  }
  os << "\n";

  os << '|' << std::string(tableWidth-1, '-') << '|' << '\n';
  // Table header
  os << "| " << std::setw(4) << "Reg" << " | " << std::setw(5) << "Alias" << " | " << std::setw(19) << "Value" << " | " << std::setw(21) << "Info" << " |\n";
  os << "|------|-------|---------------------|-----------------------|\n";

  // Register aliases and descriptions
  static constexpr const char* aliases[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
  static constexpr const char* info[] = {"Zero Register", "Return Address", "Stack Pointer", "Global Pointer", "Thread Pointer", "Temporary Register", "Temporary Register", "Temporary Register", "Callee Saved Register", "Callee Saved Register", "Arg/Return Register", "Arg/Return Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Temporary Register", "Temporary Register", "Temporary Register", "Temporary Register"};

  // Loop over the registers
  for (size_t i = 0; i < _REV_NUM_REGS_; ++i) {
    uint64_t value = RegFile->GetX<uint64_t>(Feature, i);
    
    os << "| " << std::setw(4) << ("x" + std::to_string(i));
    os << " | " << std::setw(5) << aliases[i];
    os << " | " << std::setw(19) << ("0x" + std::to_string(value));
    os << " | " << std::setw(21) << info[i] << " |\n";
  }
  os << "|" << std::string(tableWidth-1, '-') << "|" << '\n';
  return os;
}

// EOF
