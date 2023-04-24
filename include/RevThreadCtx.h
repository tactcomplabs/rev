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

/***
RevThreadCtx 

Memory Actions:
- RelinquishMem
- ShareMem
***/

#include <cstdint>
#include <vector>
#include "../include/RevMem.h"
#include "RevInstTable.h"

enum class ThreadState {
  Running,
  Ready,  
  Waiting,
  Terminated,
  Aborted,
  Retired,
};

class RevThreadCtx {

public:
  // NOTE: These Getters/Setters are currently unecessary as everything 
  //       they Get/Set is public. This may change in the future

  uint32_t GetPID() { return PID; }
  void SetPID(uint32_t NewPID) { PID = NewPID; }

  uint32_t GetParentPID() const { return ParentPID; }
  void SetParentPID(uint32_t parent_pid) { ParentPID = parent_pid; }

  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = newState; }

  RevRegFile* GetRegFile() { return RegFile; }

  uint64_t GetMemStartAddr() const { return MemInfoStartAddr; }
  void SetMemStartAddr(uint64_t newMemStartAddr) { MemInfoStartAddr = newMemStartAddr; }

  uint64_t GetMemSize() const { return MemInfoSize; }
  void SetMemSize(uint64_t newMemSize) { MemInfoSize = newMemSize; }

  bool AddChildPID(uint32_t pid);
  bool RemoveChildPID(uint32_t pid);

  bool isRunning(){ return ( State == ThreadState::Running ); }
  bool isReady(){ return (State == ThreadState::Ready); }
  bool isWaiting(){ return (State == ThreadState::Waiting); }
  bool isTerminated(){ return (State == ThreadState::Terminated); }
  bool isAborted(){ return (State == ThreadState::Aborted); }

  // FIXME: Maybe make this private? 
  uint32_t PID;
  uint32_t ParentPID;

  ThreadState State = ThreadState::Ready;
  RevRegFile *RegFile; // Actual RegFile lives in RevProc

  uint64_t MemInfoStartAddr;
  uint64_t MemInfoSize;

  std::vector<uint32_t> ChildrenPIDs = {};
};


