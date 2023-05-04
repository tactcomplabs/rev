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
  Waiting,
  Sleeping,
  Dead,
};

class RevThreadCtx {

private:
  uint32_t PID;
  uint32_t ParentPID;

  ThreadState State = ThreadState::Waiting;
  RevRegFile RegFile;

  uint64_t MemStartAddr;
  uint64_t MemSize;

  std::vector<uint32_t> ChildrenPIDs = {};

public:
  // Constructor that takes a RevRegFile object and a uint32_t ParentPID
  RevThreadCtx(const uint32_t inputPID, const RevRegFile& inputRevRegFile, uint32_t inputParentPID,
               uint64_t inputMemStartAddr, uint64_t inputMemSize )
      : PID(inputPID), ParentPID(inputParentPID), //RegFile(inputRevRegFile),
        MemStartAddr(inputMemStartAddr), MemSize(inputMemSize)
  {
    std::cout << "ADDRESS OF INCOMING REGISTER FILE = 0x"
              << std::hex << (uint64_t)(&inputRevRegFile) << std::dec << std::endl;
    std::cout << "ADDRESS OF NEW REGISTER FILE = 0x"
              << std::hex << (uint64_t)(&RegFile) << std::dec << std::endl;

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      RegFile.RV32[i] = inputRevRegFile.RV32[i];
      RegFile.RV64[i] = inputRevRegFile.RV64[i];
      RegFile.SPF[i] = inputRevRegFile.SPF[i];
      RegFile.DPF[i] = inputRevRegFile.DPF[i];

      RegFile.RV32_Scoreboard[i] = inputRevRegFile.RV32_Scoreboard[i];
      RegFile.RV64_Scoreboard[i] = inputRevRegFile.RV64_Scoreboard[i];
      RegFile.SPF_Scoreboard[i] = inputRevRegFile.SPF_Scoreboard[i];
      RegFile.DPF_Scoreboard[i] = inputRevRegFile.DPF_Scoreboard[i];
    }

    RegFile.RV64_SSTATUS = inputRevRegFile.RV64_SSTATUS;
    RegFile.RV64_SEPC    = inputRevRegFile.RV64_SEPC;
    RegFile.RV64_SCAUSE  = inputRevRegFile.RV64_SCAUSE;
    RegFile.RV64_STVAL   = inputRevRegFile.RV64_STVAL;
    RegFile.RV64_STVEC   = inputRevRegFile.RV64_STVEC;

    RegFile.RV32_SSTATUS = inputRevRegFile.RV32_SSTATUS;
    RegFile.RV32_SEPC    = inputRevRegFile.RV32_SEPC;
    RegFile.RV32_SCAUSE  = inputRevRegFile.RV32_SCAUSE;
    RegFile.RV32_STVAL   = inputRevRegFile.RV32_STVAL;
    RegFile.RV32_STVEC   = inputRevRegFile.RV32_STVEC;

    RegFile.RV32_PC = inputRevRegFile.RV32_PC;
    RegFile.RV64_PC = inputRevRegFile.RV64_PC;
    RegFile.FCSR    = inputRevRegFile.FCSR;
    RegFile.cost    = inputRevRegFile.cost;
    RegFile.trigger = inputRevRegFile.trigger;
    RegFile.Entry   = inputRevRegFile.Entry;
  }

  // RevThreadCtx&

  RevRegFile& GetRegFile() { return RegFile; }
  void SetRegFile(RevRegFile r) { RegFile = r; }

  uint32_t GetPID() { return PID; }
  void SetPID(uint32_t NewPID) { PID = NewPID; RegFile.PID = NewPID; }

  uint32_t GetParentPID() const { return ParentPID; }
  void SetParentPID(uint32_t parent_pid) { ParentPID = parent_pid; }

  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = newState; }

  uint64_t GetMemSize() { return MemSize; }
  uint64_t GetMemStartAddr() { return MemStartAddr; }

  void SetMemSize(uint64_t memSize) { MemSize = memSize; }

  void SetMemStartAddr(uint64_t memStartAddr) { MemStartAddr = memStartAddr; }

  bool AddChildPID(uint32_t pid);
  bool RemoveChildPID(uint32_t pid);

  bool isRunning(){ return ( State == ThreadState::Running ); }
  // bool isReady(){ return (State == ThreadState::Ready); }
  bool isWaiting(){ return (State == ThreadState::Waiting); }
  // bool isTerminated(){ return (State == ThreadState::Terminated); }
  bool isDead(){ return (State == ThreadState::Dead); }

};


#endif
