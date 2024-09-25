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
#include "RevRegFile.h"

namespace SST::RevCPU {

class RevThread {
public:
  ///< RevThread: Constructor
  RevThread( uint32_t ID, uint32_t ParentID, std::shared_ptr<RevMem::MemSegment> ThreadMem, std::unique_ptr<RevRegFile> RegFile )
    : ID( ID ), ParentID( ParentID ), ThreadMem( std::move( ThreadMem ) ), RegFile( std::move( RegFile ) ) {}

  ///< RevThread: Destructor
  ~RevThread() {
    // Check if any fildes are still open
    // and close them if so
    for( auto fd : fildes ) {
      if( fd > 2 ) {
        close( fd );
      }
    }
  }

  ///< RevThread: disallow copying and assignment
  RevThread( const RevThread& )            = delete;
  RevThread& operator=( const RevThread& ) = delete;

  ///< RevThread: Get this thread's ID
  uint32_t GetID() const { return ID; }

  ///< RevThread: Get the parent of this thread's TID
  uint32_t GetParentID() const { return ParentID; }

  ///< RevThread: Get the TID of the thread that this thread is waiting to join
  uint32_t GetWaitingToJoinTID() const { return WaitingToJoinTID; }

  ///< RevThread: Set the TID of the thread that this thread is waiting to join
  void SetWaitingToJoinTID( uint32_t ThreadToWaitOn ) { WaitingToJoinTID = ThreadToWaitOn; }

  ///< RevThread: Add new file descriptor to this thread (ie. rev_open)
  void AddFD( int fd ) { fildes.insert( fd ); }

  ///< RevThread: Remove file descriptor from this thread (ie. rev_close)
  void RemoveFD( int fd ) { fildes.erase( fd ); }

  ///< See if file descriptor exists/is owned by this thread
  bool FindFD( int fd ) const { return fildes.count( fd ); }

  ///< RevThread: Returns this Thread's register state
  RevRegFile* GetRegFile() const { return RegFile.get(); }

  ///< RevThread: Get this thread's current ThreadState
  ThreadState GetState() const { return State; }

  ///< RevThread: Set this thread's ThreadState
  void SetState( ThreadState newState ) { State = newState; }

  ///< RevThread: Add a child to this thread id
  void AddChildID( uint32_t tid ) { ChildrenIDs.insert( tid ); }

  ///< RevThread: Remove a child thread ID from this thread
  void RemoveChildID( uint32_t tid ) { ChildrenIDs.erase( tid ); }

  ///< RevThread: Overload the ostream printing
  friend std::ostream& operator<<( std::ostream& os, const RevThread& Thread );

  ///< RevThread: Overload the to_string method
  std::string to_string() const {
    std::ostringstream oss;
    oss << *this;      // << operator is overloaded above
    return oss.str();  // Return the string
  }

private:
  uint32_t const                      ID;                           // Thread ID
  uint32_t const                      ParentID;                     // Parent ID
  std::shared_ptr<RevMem::MemSegment> ThreadMem{};                  // TLS and Stack memory
  ThreadState                         State{ ThreadState::START };  // Thread state
  std::unique_ptr<RevRegFile> const   RegFile;                      // Register file
  std::unordered_set<uint32_t>        ChildrenIDs{};                // Child thread IDs
  std::unordered_set<int>             fildes{ 0, 1, 2 };            // Default file descriptors

  ///< RevThread: ID of the thread this thread is waiting to join
  uint32_t WaitingToJoinTID = _INVALID_TID_;

};  // class RevThread

}  // namespace SST::RevCPU

#endif
