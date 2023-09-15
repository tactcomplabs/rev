//
// _RevThreadCtx_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#ifndef _SST_REVCPU_REVTHREADCTX_H_
#define _SST_REVCPU_REVTHREADCTX_H_

// -- Standard Headers
#include <cstdint>
#include <vector>
#include <set>

// -- Rev Headers
#include "RevMem.h"
#include "RevInstTable.h"

namespace SST::RevCPU{

/// RevThreadCtx: Enum for tracking state of a software thread (Unused)
enum class ThreadState {
  Running,
  Waiting,
  Sleeping,
  Dead,
};

class RevThreadCtx {
  uint32_t PID;                                               /// Software PID of thread
  uint32_t ParentPID;                                         /// Parent Ctx's PID
  ThreadState State = ThreadState::Waiting;                   /// Thread state
  RevRegFile RegFile{};                                       /// Each context has its own register file
  std::set<uint32_t> ChildrenPIDs;                            /// List of a thread's children
  std::set<int> fildes = {0, 1, 2};                           /// Initial fildes are STDOUT, STDIN, and STDERR

public:
  // Constructor that takes a RevRegFile object and a uint32_t ParentPID
  RevThreadCtx(const uint32_t inputPID,  const uint32_t inputParentPID)
      : PID(inputPID), ParentPID(inputParentPID) {
  }

  void AddFD(int fd) { fildes.insert(fd); }                   /// RevThreadCtx: Add fd to Ctx's fildes
  bool RemoveFD(int fd) { return fildes.erase(fd); }          /// RevThreadCtx: Remove fd to Ctx's fildes
  bool FindFD(int fd) { return fildes.count(fd); }            /// RevThreadCtx: See if Ctx has ownership of fd
  auto GetFildes(){ return fildes; }                          /// RevThreadCtx: Get list of file descriptors owned by Ctx

  bool DuplicateRegFile(const RevRegFile& regToDup);          /// RevThreadCtx: Makes its own register file a copy of regToDup
  RevRegFile* GetRegFile() { return &RegFile; }               /// RevThreadCtx: Returns pointer to its register file
  void SetRegFile(RevRegFile r) { RegFile = r; }              /// RevThreadCtx: Sets pointer to its register file

  auto GetPID() { return PID; }                               /// RevThreadCtx: Gets Ctx's PID
  void SetPID(uint32_t NewPID) { PID = NewPID; }              /// RevThreadCtx: Sets Ctx's PID

  auto GetParentPID() const { return ParentPID; }                    /// RevThreadCtx: Gets Ctx's Parent's PID
  void SetParentPID(uint32_t parent_pid) { ParentPID = parent_pid; } /// RevThreadCtx: Sets Ctx's Parent's PID

  ThreadState GetState() const { return State; }                     /// RevThreadCtx: Returns the state (ThreadState) of this Ctx
  void SetState(ThreadState newState) { State = newState; }          /// RevThreadCtx: Used to change ThreadState of this Ctx

  bool AddChildPID(uint32_t pid) { return ChildrenPIDs.insert(pid).second; } /// RevThreadCtx: Adds a child Ctx's pid to this ones children vector
  bool RemoveChildPID(uint32_t pid) { return ChildrenPIDs.erase(pid); }      /// RevThreadCtx: Removes a child Ctx's pid to this ones children vector

  bool isRunning(){ return State == ThreadState::Running; }          /// RevThreadCtx: Checks if Ctx's ThreadState is Running
  bool isWaiting(){ return State == ThreadState::Waiting; }          /// RevThreadCtx: Checks if Ctx's ThreadState is Running
  bool isDead(){    return State == ThreadState::Dead;    }          /// RevThreadCtx: Checks if Ctx's ThreadState is Running
};

} // namespace SST::RevCPU
#endif
