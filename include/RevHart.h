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
  uint16_t ID;

  ///< RevHart: State management object when a Hart is executing a system call
  std::unique_ptr<EcallState> Ecall;

  ///< RevHart: Hart is ready decode next instruction
  bool ReadyToDecode;
  
  ///< RevHart: Hart is ready to execute its decoded inst (ie. no depedenecies)
  bool ReadyToExecute; 

  ///< RevHart: Thread currently executing on this Hart
  uint32_t AssignedThreadID;

public:
  ///< RevHart: Constructor
  RevHart(uint16_t id) 
      : ID(id), 
        Ecall(std::make_unique<EcallState>()), 
        ReadyToDecode(false), 
        ReadyToExecute(false), 
        AssignedThreadID(_INVALID_TID_) {
  }

  ///< RevHart: Destructor
  ~RevHart(){}

  ///< RevHart: Get the EcallState
  EcallState& GetEcallState(){ return *Ecall; }

  ///< RevHart: Returns if a Hart is ready to decode its next instruction (CTS)
  bool isReadyToDecode(){ return ReadyToDecode && (AssignedThreadID != _INVALID_TID_); }
  
  ///< RevHart: Returns if a Hart is ready to execute the decoded instruction (CTE)
  bool isReadyToExecute() const { return ReadyToExecute && (AssignedThreadID != _INVALID_TID_); }

  // ReadyToDecode == CTS
  void SetReadyToDecode(bool rtd ){ ReadyToDecode = rtd; }

  // ReadyToExecute == CTE
  void SetReadyToExecute(bool rte){ ReadyToExecute = rte; }

  ///< RevHart: Get Hart's ID 
  const uint16_t& GetID(){ return ID; }

  ///< RevHart: Returns true if a thread is assigned to this Hart
  bool HasThread(){ return AssignedThreadID != _INVALID_TID_; }

  ///< RevHart: Returns the ThreadID of the assigned thread
  /// TODO: Potentially move the checking logic here for if a thread is not assigned? Right now it's handled elsewhere
  uint32_t GetAssignedThreadID(){ return AssignedThreadID; }

  ///< RevHart: Assigns a RevThread to this Hart
  void AssignThread(const uint32_t& ThreadID){ AssignedThreadID = ThreadID; }

  ///< RevHart: Removed a RevThread from this Hart
  // TODO: Maybe check if Thread has any reasons it can't be removed (ie. Outstanding Load)
  void UnassignThread(){ AssignedThreadID = _INVALID_TID_; }

};

//EcallStatus RevHart::EcallLoadAndParseString(...){
//    if(Ecall.buf) {
//      // do something with Hart (*this). Its members can be written as ordinary variables.
//    }
//}
};
#endif
