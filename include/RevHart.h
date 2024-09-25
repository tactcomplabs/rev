//
// _RevHart_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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

namespace SST::RevCPU {

class RevHart {
  ///< RevHart: Id for the Hart (0,1,2,3,etc)
  unsigned const ID;

  ///< RevHart: State management object when a Hart is executing a system call
  EcallState Ecall{};

  ///< RevHart: Pointer to the Proc's LSQueue
  const std::shared_ptr<std::unordered_multimap<uint64_t, MemReq>>& LSQueue;

  ///< RevHart: Pointer to the Proc's MarkLoadCompleteFunc
  std::function<void( const MemReq& )> const MarkLoadCompleteFunc;

  ///< RevHart: Thread currently executing on this Hart
  std::unique_ptr<RevThread> Thread = nullptr;

public:
  ///< RevHart: Constructor
  RevHart(
    unsigned                                                          ID,
    const std::shared_ptr<std::unordered_multimap<uint64_t, MemReq>>& LSQueue,
    std::function<void( const MemReq& )>                              MarkLoadCompleteFunc
  )
    : ID( ID ), LSQueue( LSQueue ), MarkLoadCompleteFunc( std::move( MarkLoadCompleteFunc ) ) {}

  ///< RevHart: disallow copying and assignment
  RevHart( const RevHart& )            = delete;
  RevHart& operator=( const RevHart& ) = delete;

  ///< RevHart: Destructor
  ~RevHart()                           = default;

  ///< RevHart: Get the EcallState
  EcallState& GetEcallState() { return Ecall; }

  ///< RevHart: Get the RegFile
  RevRegFile* GetRegFile() const { return Thread->GetRegFile(); }

  ///< RevHart: Add new file descriptor to this hart's thread (ie. rev_open)
  void AddFD( int fd ) { Thread->AddFD( fd ); }

  ///< RevHart: Remove file descriptor from this hart's thread (ie. rev_close)
  void RemoveFD( int fd ) { Thread->RemoveFD( fd ); }

  ///< See if file descriptor exists/is owned by this hart's thread
  bool FindFD( int fd ) const { return Thread->FindFD( fd ); }

  ///< RevHart: Get Hart's ID
  uint16_t GetID() const { return ID; }

  ///< RevHart: Returns the ID of the assigned thread
  uint32_t GetThreadID() const { return Thread ? Thread->GetID() : _INVALID_TID_; }

  ///< RevHart: Assigns a RevThread to this Hart
  void SetThread( std::unique_ptr<RevThread> ThreadToAssign ) {
    Thread       = std::move( ThreadToAssign );
    auto RegFile = Thread->GetRegFile();
    RegFile->SetMarkLoadComplete( MarkLoadCompleteFunc );
    RegFile->SetLSQueue( LSQueue );
    Thread->SetState( ThreadState::RUNNING );
  }

  ///< RevHart: Remove a RevThread from this Hart
  std::unique_ptr<RevThread> PopThread() { return std::move( Thread ); }
};  // class RevHart

}  // namespace SST::RevCPU
#endif
