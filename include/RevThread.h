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

// -- Standard Headers
#include <cstdint>
#include <set>
#include <unistd.h>
#include <vector>

// -- Rev Headers
#include "RevInstTable.h"
#include "RevMem.h"

namespace SST::RevCPU{

// Enum for tracking the state of a RevThread.
enum class ThreadState {
  START,    // Initial state; RevCPU has not yet registered it in its Threads table
  RUNNING,  // Thread is assigned and executing on a Proc/HART (NOTE: This does NOT change if a Thread is stalled)
  BLOCKED,  // Waiting for thread synchronization at this point (Currently only triggered by call to `rev_pthread_join`)
  READY,    // In the ready list; waiting for CPU (not implemented yet).
  DONE,     // Thread has finished; deallocate resources.
};

class RevThread {
  uint32_t ThreadID;                                   // Thread ID
  uint32_t ParentThreadID;                             // Parent ThreadID
  uint64_t StackPtr;                                   // Initial stack pointer
  uint64_t FirstPC;                                    // Initial PC
  std::shared_ptr<RevMem::MemSegment> ThreadMem;       // TLS and Stack memory
  RevFeature* Feature;                                 // Feature set for this thread
  uint64_t ThreadPtr;                                  // Thread pointer
  ThreadState State = ThreadState::START;              // Thread state (Initializes as )
  RevRegFile RegFile;                                  // Register file
  std::unordered_set<uint32_t> ChildrenThreadIDs = {}; // Child thread IDs
  std::unordered_set<int> fildes = {0, 1, 2};          // Default file descriptors

  uint32_t WaitingToJoinTID = _INVALID_TID_;

public:
  RevThread(uint32_t ThreadID, uint32_t ParentThreadID,
            uint64_t StackPtr, uint64_t FirstPC,
            std::shared_ptr<RevMem::MemSegment>& ThreadMem,
            RevFeature* Feature)
    : ThreadID(ThreadID), ParentThreadID(ParentThreadID), StackPtr(StackPtr),
      FirstPC(FirstPC), ThreadMem(ThreadMem), Feature(Feature), RegFile(Feature){
    // Set the stack pointer
    RegFile.SetX(RevReg::sp, StackPtr);

    // Set the thread pointer
    ThreadPtr = ThreadMem->getTopAddr();
    RegFile.SetX(RevReg::tp, ThreadPtr);

    // Set the PC
    RegFile.SetPC(FirstPC);
  }

  ~RevThread(){
    // Check if any fildes are still open
    // and close them if so
    for(auto fd : fildes){
      if(fd > 2){
       close(fd);
      }
    }
  }

  // ThreadID operations
  uint32_t GetThreadID() const { return ThreadID; }
  void SetThreadID(const uint32_t NewThreadID) { ThreadID = NewThreadID; }
  uint32_t GetParentThreadID() const { return ParentThreadID; }
  void SetParentThreadID(const uint32_t NewParentThreadID) { ParentThreadID = NewParentThreadID; }
  uint32_t GetWaitingToJoinTID() const { return WaitingToJoinTID; }
  void SetWaitingToJoinTID(const uint32_t ThreadToWaitOn) { WaitingToJoinTID = ThreadToWaitOn; }

  // Add new file descriptor
  void AddFD(int fd){ fildes.insert(fd); }

  // Remove file descriptor from this thread (ie. rev_close)
  void RemoveFD(int fd){ fildes.erase(fd); }

  // See if file descriptor exists/is owned by this thread
  bool FindFD(int fd){ return fildes.count(fd); }

  // Get the threads
  const std::unordered_set<int>& GetFildes(){ return fildes; }

  // Register file operations
  const RevRegFile* GetRegFile() const { return &RegFile; }
  RevRegFile* GetRegFile(){ return &RegFile; }

  // RevFeature operations
  RevFeature* GetFeature() const { return Feature; }
  void SetFeature(RevFeature* r);

  // Thread state operations
  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = std::move(newState); }

  // Add a child to this thread id
  void AddChildThreadID(uint32_t tid){ ChildrenThreadIDs.insert(tid); }

  // Remove a child thread ID from this thread
  void RemoveChildThreadID(uint32_t tid){ ChildrenThreadIDs.erase(tid); }

  // State checks
  bool isRunning();
  bool isBlocked();
  bool isDone();

// Overload the printing
  friend std::ostream& operator<<(std::ostream& os, const RevThread& Thread);

  std::string to_string() const {
    std::ostringstream oss;
    oss << *this;      // << operator is overloaded above
    return oss.str();  // Return the string
  }

}; // class RevThread

} // namespace SST::RevCPU

#endif
