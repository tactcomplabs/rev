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

class RevThread {
public:
  using RevVirtRegState = RevRegFile;

  /////< RevThread: Constructor
  //RevThread(uint32_t ID, uint32_t ParentID,
  //          uint64_t StackPtr, uint64_t FirstPC,
  //          std::shared_ptr<RevMem::MemSegment>& ThreadMem,
  //          RevFeature* Feature)
  //  : ID(ID), ParentID(ParentID), StackPtr(StackPtr),
  //    FirstPC(FirstPC), ThreadMem(ThreadMem), Feature(Feature) {
  //  // Create the register file
  //  VirtRegState = std::make_unique<RevVirtRegState>(Feature);
  //  // Set the stack pointer
  //  VirtRegState->SetX(RevReg::sp, StackPtr);

  //  // Set the thread pointer
  //  ThreadPtr = ThreadMem->getTopAddr();
  //  VirtRegState->SetX(RevReg::tp, ThreadPtr);

  //  // Set the PC
  //  VirtRegState->SetPC(FirstPC);
  //}

  RevThread(uint32_t ID, uint32_t ParentID,
            std::shared_ptr<RevMem::MemSegment>& ThreadMem,
            std::unique_ptr<RevVirtRegState> VirtRegState)
    : ID(ID), ParentID(ParentID),
      ThreadMem(ThreadMem), VirtRegState(std::move(VirtRegState)){
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

  // ID operations
  uint32_t GetID() const { return ID; }
  void SetID(const uint32_t NewID) { ID = NewID; }
  uint32_t GetParentID() const { return ParentID; }
  void SetParentID(const uint32_t NewParentID) { ParentID = NewParentID; }
  uint32_t GetWaitingToJoinTID() const { return WaitingToJoinTID; }
  void SetWaitingToJoinTID(const uint32_t ThreadToWaitOn) { WaitingToJoinTID = ThreadToWaitOn; }

  // Add new file descriptor
  void AddFD(int fd){ fildes.insert(fd); }

  // Remove file descriptor from this thread (ie. rev_close)
  void RemoveFD(int fd){ fildes.erase(fd); }

  // See if file descriptor exists/is owned by this thread
  bool FindFD(int fd){ return fildes.count(fd); }


  // TODO: Potentially move these to RevHart and have them set it when it's assigned?
  // Arguments could be made to keep it here or put it there
  void SetLSQueue(std::shared_ptr<std::unordered_map<uint64_t, MemReq>> lsq){
    // TODO: Ask dave if this is right
    VirtRegState->SetLSQueue(lsq);
  }

  void SetMarkLoadComplete(std::function<void(const MemReq&)> func){
    // TODO: Ask dave if this is right
    VirtRegState->SetMarkLoadComplete(func);
  }

  // Get the threads
  const std::unordered_set<int>& GetFildes(){ return fildes; }

  // Set the register file
  void UpdateVirtRegState(std::unique_ptr<RevVirtRegState> vRegState){ VirtRegState = std::move(vRegState); }

  std::unique_ptr<RevVirtRegState> TransferVirtRegState(){ return std::move(VirtRegState); }

  // RevFeature operations
  RevFeature* GetFeature() const { return Feature; }
  void SetFeature(RevFeature* r);

  // Thread state operations
  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = std::move(newState); }

  // Add a child to this thread id
  void AddChildID(uint32_t tid){ ChildrenIDs.insert(tid); }

  // Remove a child thread ID from this thread
  void RemoveChildID(uint32_t tid){ ChildrenIDs.erase(tid); }

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
private:
  uint32_t ID;                                         // Thread ID
  uint32_t ParentID;                                   // Parent ID
  uint64_t StackPtr;                                   // Initial stack pointer
  uint64_t FirstPC;                                    // Initial PC
  std::shared_ptr<RevMem::MemSegment> ThreadMem;       // TLS and Stack memory
  RevFeature* Feature;                                 // Feature set for this thread
  uint64_t ThreadPtr;                                  // Thread pointer
  ThreadState State = ThreadState::START;              // Thread state (Initializes as )
  std::unique_ptr<RevVirtRegState> VirtRegState;       // Register file
  std::unordered_set<uint32_t> ChildrenIDs = {}; // Child thread IDs
  std::unordered_set<int> fildes = {0, 1, 2};          // Default file descriptors

  ///< RevThread: ID of the thread this thread is waiting to join
  uint32_t WaitingToJoinTID = _INVALID_TID_;

}; // class RevThread

} // namespace SST::RevCPU

#endif
