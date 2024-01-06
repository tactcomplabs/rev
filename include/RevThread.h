//
// _RevThread_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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

  ///< RevThread: Constructor
  RevThread(uint32_t ID, uint32_t ParentID,
            std::shared_ptr<RevMem::MemSegment>& ThreadMem,
            std::unique_ptr<RevVirtRegState> VirtRegState)
    : ID(ID), ParentID(ParentID),
      ThreadMem(ThreadMem), VirtRegState(std::move(VirtRegState)){}

  ///< RevThread: Destructor
  ~RevThread(){
    // Check if any fildes are still open
    // and close them if so
    for(auto fd : fildes){
      if(fd > 2){
       close(fd);
      }
    }
  }

  ///< RevThread: Get this thread's ID
  uint32_t GetID() const { return ID; }

  ///< RevThread: Set this thread's ID
  void SetID(const uint32_t NewID) { ID = NewID; }

  ///< RevThread: Get the parent of this thread's TID
  uint32_t GetParentID() const { return ParentID; }

  ///< RevThread: Set the parent of this thread's TID
  void SetParentID(const uint32_t NewParentID) { ParentID = NewParentID; }

  ///< RevThread: Get the TID of the thread that this thread is waiting to join
  uint32_t GetWaitingToJoinTID() const { return WaitingToJoinTID; }

  ///< RevThread: Set the TID of the thread that this thread is waiting to join
  void SetWaitingToJoinTID(const uint32_t ThreadToWaitOn) { WaitingToJoinTID = ThreadToWaitOn; }

  ///< RevThread: Add new file descriptor to this thread (ie. rev_open)
  void AddFD(int fd){ fildes.insert(fd); }

  ///< RevThread: Remove file descriptor from this thread (ie. rev_close)
  void RemoveFD(int fd){ fildes.erase(fd); }

  ///< See if file descriptor exists/is owned by this thread
  bool FindFD(int fd){ return fildes.count(fd); }

  ///< RevThread: Get the fildes valid for this thread
  const std::unordered_set<int>& GetFildes(){ return fildes; }

  ///< RevThread: Update this thread's virtual register state
  void UpdateVirtRegState(std::unique_ptr<RevVirtRegState> vRegState){
    VirtRegState = std::move(vRegState);
  }

  ///< RevThread: Transfers the pointer of this Thread's register state
  ///             (Used for loading reg state into a Hart's RegFile)
  std::unique_ptr<RevVirtRegState> TransferVirtRegState(){
    return std::move(VirtRegState);
  }

  ///< RevThread: Get the RevFeature this thread was created with
  RevFeature* GetFeature() const { return Feature; }

  ///< RevThread: Get this thread's current ThreadState
  ThreadState GetState() const { return State; }

  ///< RevThread: Set this thread's ThreadState
  void SetState(ThreadState newState) { State = std::move(newState); }

  ///< RevThread: Add a child to this thread id
  void AddChildID(uint32_t tid){ ChildrenIDs.insert(tid); }

  ///< RevThread: Remove a child thread ID from this thread
  void RemoveChildID(uint32_t tid){ ChildrenIDs.erase(tid); }

  ///< RevThread: Overload the ostream printing
  friend std::ostream& operator<<(std::ostream& os, const RevThread& Thread);

  ///< RevThread: Overload the to_string method
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
  ThreadState State = ThreadState::START;              // Thread state
  std::unique_ptr<RevVirtRegState> VirtRegState;       // Register file
  std::unordered_set<uint32_t> ChildrenIDs = {};       // Child thread IDs
  std::unordered_set<int> fildes = {0, 1, 2};          // Default file descriptors

  ///< RevThread: ID of the thread this thread is waiting to join
  uint32_t WaitingToJoinTID = _INVALID_TID_;

}; // class RevThread

} // namespace SST::RevCPU

#endif
