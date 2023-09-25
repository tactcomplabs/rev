//
// _RevThread_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#ifndef _SST_REVCPU_REVTHREAD_H_
#define _SST_REVCPU_REVTHREAD_H_

#define __INVALID_TID__ 0x0

// -- Standard Headers
#include <cstdint>
#include <set>
#include <vector>

// -- Rev Headers
#include "RevInstTable.h"
#include "RevMem.h"

using namespace SST::RevCPU;
using MemSegment = RevMem::MemSegment;

// Enum for tracking the state of a RevThread.
enum class ThreadState {
  START,    // Initial state; resources are allocated.
  RUNNING,  // Thread is assigned and executing on a Proc/HART.
  BLOCKED,  // Waiting for I/O or thread synchronization.
  READY,    // In the ready list; waiting for CPU (not implemented yet).
  DONE,     // Thread has finished; deallocate resources.
};

class RevThread {
 public:
  RevThread(uint32_t inputThreadID, uint32_t inputParentThreadID,
            uint64_t inputStackPtr, uint64_t inputFirstPC,
            std::shared_ptr<MemSegment>& inputThreadMem,
            RevFeature* inputFeature);

  // ThreadID operations
  uint32_t GetThreadID() { return ThreadID; }
  void SetThreadID(uint32_t NewThreadID) { ThreadID = NewThreadID; }
  uint32_t GetParentThreadID() { return ParentThreadID; }
  void SetParentThreadID(uint32_t NewParentThreadID) {
    ParentThreadID = NewParentThreadID;
  }
  const uint32_t& GetWaitingToJoinTID() { return WaitingToJoinTID; }
  void SetWaitingToJoinTID(uint32_t ThreadToWaitOn) {
    WaitingToJoinTID = ThreadToWaitOn;
  }

  // File descriptor operations
  void AddFD(int fd);
  bool RemoveFD(int fd);
  bool FindFD(int fd);
  std::vector<int> GetFildes();

  // Register file operations
  RevRegFile* GetRegFile() { return &RegFile; }
  void SetRegFile(RevRegFile r);

  // RevFeature operations
  RevFeature* GetFeature() { return Feature; }
  void SetFeature(RevFeature* r);

  // Thread state operations
  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = newState; }

  // Child thread operations
  bool AddChildThreadID(uint32_t pid);
  bool RemoveChildThreadID(uint32_t pid);

  // State checks
  bool isRunning();
  bool isBlocked();
  bool isDone();

  // Printing functions
  std::string GetStateString();
  friend std::ostream& operator<<(std::ostream& os, RevThread& Thread);

  const char* print() {
    std::ostringstream oss;
    oss << *this;  // Assuming operator<< is already overloaded for RevThread
    SSTOutputBuffer = oss.str();     // Store the string
    return SSTOutputBuffer.c_str();  // Return C-string
  }

 private:
  uint32_t ThreadID;                       // Thread ID
  uint32_t ParentThreadID;                 // Parent ThreadID
  uint64_t StackPtr;                       // Initial stack pointer
  uint64_t FirstPC;                        // Initial PC
  std::shared_ptr<MemSegment> ThreadMem;   // TLS and Stack memory
  RevFeature* Feature;                     // Feature set for this thread
  uint64_t ThreadPtr;                      // Thread pointer
  ThreadState State = ThreadState::START;  // Thread state (Initializes as )
  RevRegFile RegFile;                      // Register file
  std::vector<uint32_t> ChildrenThreadIDs = {};
  std::vector<int> fildes = {0, 1, 2};  // Default file descriptors

  uint32_t WaitingToJoinTID = __INVALID_TID__;

  std::string SSTOutputBuffer;  // Buffer to hold the printed string
};
#endif
