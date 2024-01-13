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
#include "RevNIC.h"
#include "RevNOC.h"
#include "SST.h"

namespace SST::RevCPU{

class RevHart{

  ///< RevHart: Id for the Hart (0,1,2,3,etc)
  unsigned ID;

  ///< RevHart: State management object when a Hart is executing a system call
  EcallState Ecall{};

  ///< RevHart: Pointer to the Proc's LSQueue
  const std::shared_ptr<std::unordered_map<uint64_t, MemReq>>& LSQueue;

  ///< RevHart: Pointer to the Proc's MarkLoadCompleteFunc
  std::function<void(const MemReq&)> MarkLoadCompleteFunc;

  ///< RevHart: Pointer to the SST output object
  SST::Output *output = nullptr;

  ///< RevHart: Thread currently executing on this Hart
  std::unique_ptr<RevThread> Thread = nullptr;
  std::unique_ptr<RevRegFile> RegFile = nullptr;

  ///< RevHart: NIC interface for this Hart
  RevNicAPI* NIC = nullptr;

  ///< RevHart: NOC interface for this Hart
  RevNocAPI* NOC = nullptr;

  ///< RevHart: Pointer to a message handler for this hart if you assign a nic to it
  void NICMsgHandler(Event *ev);

  ///< RevHart: Pointer to a message handler for this hart if you assign a nic to it
  void NOCMsgHandler(Event *ev);

  ///< RevHart: Make RevProc a friend of this
  friend class RevProc;

public:
  ///< RevHart: Constructor
  RevHart(unsigned ID, const std::shared_ptr<std::unordered_map<uint64_t, MemReq>>& LSQueue,
          std::function<void(const MemReq&)> MarkLoadCompleteFunc, SST::Output *output)
    : ID(ID), LSQueue(LSQueue), MarkLoadCompleteFunc(std::move(MarkLoadCompleteFunc)), output(output) {}

  ///< RevHart: Destructor (delete NIC if it exists)
  ~RevHart() = default;

  ///< RevHart: Get the EcallState
  EcallState& GetEcallState() { return Ecall; }
  const EcallState& GetEcallState() const { return Ecall; }

  ///< RevHart: Get Hart's ID
  uint16_t GetID() const { return ID; }

  ///< RevHart: Returns the ID of the assigned thread
  uint32_t GetAssignedThreadID() const { return (Thread != nullptr) ? Thread->GetID() : _INVALID_TID_; }

  ///< RevHart: Load the register file from the RevThread
  void LoadRegFile(std::unique_ptr<RevRegFile> regFile){
    RegFile = std::move(regFile);
    RegFile->SetMarkLoadComplete(MarkLoadCompleteFunc);
    RegFile->SetLSQueue(LSQueue);
  }

  ///< RevHart: Assigns a RevThread to this Hart
  void AssignThread(std::unique_ptr<RevThread> ThreadToAssign){
    Thread = std::move(ThreadToAssign);
    Thread->SetState(ThreadState::RUNNING);
    LoadRegFile(Thread->TransferVirtRegState());
  }

  ///< RevHart: Removed a RevThread from this Hart
  std::unique_ptr<RevThread> PopThread(){
    // return the register file to the thread
    Thread->UpdateVirtRegState(std::move(RegFile));
    // return the thread
    return std::move(Thread);
  }

  ///< RevHart: Assign a NIC to this Core / optionally Hart
  ///           (Called from RevCPU)
  void AssignNIC(RevNicAPI* nic);

  ///< RevHart: Give's access to an 'external' NIC
  ///           The key caveat here over the AssignNIC function is that
  ///           this Hart doesn't override the messageHandler of the NIC
  ///           Only use this is you want certain harts to have certain NICs
  ///           (Called from RevProc)
  void GiveAccessToNIC(RevNicAPI* nic);

  ///< RevHart: Assigns NOC interface to this Hart
  ///           This function overrides the messageHandler of the NOC
  ///           and only this Hart can receive messages from this NOC interface
  void AssignNOC(RevNocAPI* noc);

  ///< RevHart: Give's access to an 'external' NOC interface
  ///           (Doesn't override the messageHandler)
  void GiveAccessToNOC(RevNocAPI* noc);

}; // class RevHart

} // namespace SST::RevCPU
#endif
