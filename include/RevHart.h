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
  std::unique_ptr<EcallState> Ecall;

  ///< RevHart: Thread currently executing on this Hart
  uint32_t AssignedThreadID;

public:
  ///< RevHart: Constructor
  RevHart(uint16_t id) 
      : ID(id), 
        Ecall(std::make_unique<EcallState>()), 
        AssignedThreadID(_INVALID_TID_) {
  }

  ///< RevHart: Destructor
  ~RevHart(){}

  ///< RevHart: Get the EcallState
  EcallState& GetEcallState() { return *Ecall; }

  ///< RevHart: Get the EcallState
  EcallState& GetEcallState() const { return *Ecall; }

  ///< RevHart: Get Hart's ID 
  uint16_t GetID() const { return ID; }

  ///< RevHart: Returns the ThreadID of the assigned thread
  uint32_t GetAssignedThreadID() const { return AssignedThreadID; }

  ///< RevHart: Assigns a RevThread to this Hart
  void AssignThread(const uint32_t& ThreadID){ AssignedThreadID = ThreadID; }

  ///< RevHart: Removed a RevThread from this Hart
  void UnassignThread(){ AssignedThreadID = _INVALID_TID_; }

};

};
#endif
