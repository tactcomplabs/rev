//
// _RevHart_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVHART_H_
#define _SST_REVCPU_REVHART_H_

// -- SST Headers
#include "RevSysCalls.h"
#include "RevThread.h"
#include "SST.h"

namespace SST::RevCPU{

class RevHart{
  ///< RevHart: Id for the Hart (0,1,2,3,etc)
  unsigned ID;

  ///< RevHart: State management object when a Hart is executing a system call
  EcallState Ecall{};

  ///< RevHart: Thread currently executing on this Hart
  std::unique_ptr<RevThread> Thread = nullptr;
  std::unique_ptr<RevRegFile> RegFile = nullptr;

  ///< RevHart: Make this a friend of RevThread
  friend class RevProc;

public:
  ///< RevHart: Constructor
  RevHart(uint16_t id) : ID(id) { }

  typedef std::unique_ptr<RevHart> ptr;

  ///< RevHart: Destructor
  ~RevHart() = default;

  ///< RevHart: Get the EcallState
  EcallState& GetEcallState() { return Ecall; }
  const EcallState& GetEcallState() const { return Ecall; }

  ///< RevHart: Get Hart's ID
  uint16_t GetID() const { return ID; }

  ///< RevHart: Returns the ThreadID of the assigned thread
  uint32_t GetAssignedThreadID() const { return (Thread != nullptr) ? Thread->GetThreadID() : _INVALID_TID_; }

  ///< RevHart: Load the register file from the RevThread
  void LoadRegFile(std::unique_ptr<RevRegFile> regFile){
    RegFile = std::move(regFile);
  }

  ///< RevHart: Assigns a RevThread to this Hart
  void AssignThread(std::unique_ptr<RevThread> ThreadToAssign){
    Thread = std::move(ThreadToAssign);
    LoadRegFile(Thread->TransferRegState());
  }

  ///< RevHart: Removed a RevThread from this Hart
  std::unique_ptr<RevThread> PopThread(){
    // return the register file to the thread
    Thread->UpdateRegState(std::move(RegFile));
    // return the thread
    return std::move(Thread);
  }
}; // class RevHart

} // namespace SST::RevCPU
#endif
