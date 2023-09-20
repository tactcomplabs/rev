//
// _RevThread_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#ifndef _SST_REVCPU_REVTHREAD_H_
#define _SST_REVCPU_REVTHREAD_H_

#define __INVALID_TID__ 0x0

// -- Standard Headers
#include <algorithm>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
#include <vector>

// -- Rev Headers
#include "RevMem.h"
#include "RevInstTable.h"

using MemSegment = SST::RevCPU::RevMem::MemSegment;

/// RevThread: Enum for tracking state of a software thread (Unused)
enum class ThreadState {
  START,    // Allocate Resources 
  RUNNING,  // Has the CPU 
  BLOCKED,  // Waiting for I/O OR synchronization with another thread (mutex, condition variable, sempahore)
  READY,    // On the ready list (not implemented yet) waiting for CPU availability 
  DONE,     // Deallocate resources
};

class RevThread {
  using RevRegFile = SST::RevCPU::RevRegFile;
public:
    // Constructor that takes a RevRegFile object and a uint32_t ParentThreadID
    RevThread( uint32_t ParentThreadID,
               uint64_t inputStackPtr,
               uint64_t inputFirstPC,
               std::shared_ptr<MemSegment>& inputThreadMem);

    void SetTIDAddr(uint64_t tidAddr){ ThreadIDAddr = tidAddr; } /// RevThread: Sets the address of the ThreadID
    uint64_t GetTIDAddr(){ return ThreadIDAddr; }   /// RevThread: Gets the address of the ThreadID
    void AddFD(int fd);                             /// RevThread: Add fd to Thread's fildes 
    bool RemoveFD(int fd);                          /// RevThread: Remove fd to Thread's fildes 
    bool FindFD(int fd);                            /// RevThread: See if Thread has ownership of fd
    std::vector<int> GetFildes(){ return fildes; }  /// RevThread: Get list of file descriptors owned by Thread

    RevRegFile* GetRegFile() { return &RegFile; }   /// RevThread: Returns pointer to its register file
    void SetRegFile(RevRegFile r) { RegFile = r; }  /// RevThread: Sets pointer to its register file

    uint32_t GetThreadID() { return ThreadID; }                                  /// RevThread: Gets Thread's ThreadID
    void SetThreadID(uint32_t NewThreadID) { ThreadID = NewThreadID; }           /// RevThread: Sets Thread's ThreadID

    uint32_t GetParentThreadID() const { return ParentThreadID; }                /// RevThread: Gets Thread's Parent's ThreadID
    void SetParentThreadID(uint32_t parent_pid) { ParentThreadID = parent_pid; } /// RevThread: Gets Thread's ThreadID

    ThreadState GetState() const { return State; }                     /// RevThread: Returns the state (ThreadState) of this Thread
    void SetState(ThreadState newState) { State = newState; }          /// RevThread: Used to change ThreadState of this Thread

    bool AddChildThreadID(uint32_t pid);                               /// RevThread: Adds a child Thread's pid to this ones children vector
    bool RemoveChildThreadID(uint32_t pid);                            /// RevThread: Removes a child Thread's pid to this ones children vector

    bool isRunning(){ return ( State == ThreadState::RUNNING ); }      /// RevThread: Checks if Thread's ThreadState is Running
    bool isBlocked(){ return (State == ThreadState::BLOCKED); }        /// RevThread: Checks if Thread's ThreadState is 
    bool isDone(){ return (State == ThreadState::DONE); }                /// RevThread: Checks if Thread's ThreadState is Done

    const uint32_t& GetWaitingToJoinTID(){ return WaitingToJoinTID; }
    void SetWaitingToJoinTID(uint32_t ThreadToWaitOn){ WaitingToJoinTID = ThreadToWaitOn; }

    // Override the printing 
    friend std::ostream& operator<<(std::ostream& os, RevThread& Thread) {
      os << "Thread " << Thread.ThreadID << ":" << std::endl; 
      os << "\tThreadMem = " << *Thread.ThreadMem << std::endl;
      os << std::hex << std::internal; // set hex output & internal padding 
      os << "\tRV64_PC = 0x" << std::setw(16) << std::setfill('0') << Thread.RegFile.RV64_PC << " | RV32_PC = 0x" << std::setw(8) << std::setfill('0') << Thread.RegFile.RV32_PC << std::endl;
      for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
        if( i < 10){ os << " "; } 
        os << "\t";
        os << "RV32[" << std::dec << std::setw(2) << std::setfill(' ') << i << "]: 0x" << std::setw(8) << std::setfill('0') << std::hex << Thread.RegFile.RV32[i]; 
        os << " | "; 
        os << "RV64[" << std::dec << std::setw(2) << std::setfill(' ') << i << "]: 0x" << std::setw(16) << std::setfill('0') << std::hex << Thread.RegFile.RV64[i]; 
        switch (i) {
          case 1:
            os << " --- Return Address";
            break;
          case 2:
            os << " --- Stack Pointer";
            break;
          case 3:
            os << " --- Global Pointer";
            break;
          case 4: 
            os << " --- Thread Pointer";
            break;
        default:
          if( i >= 5 && i <= 7 ){
            os << " --- t" << std::dec << i-5 << " (Temporary Register)";
          }
          else if( i >= 8 && i <= 9 ){
            os << " --- s" << std::dec << i-8 << " (Callee Saved Register)";
          }
          else if( i >= 10 && i <= 17 ){
           os << " --- a" << std::dec <<  i-10 << " (Argument Register)";
          } 
          else if ( i >= 18 && i <= 27 ){
            os << " --- s" << std::dec << i-16 << " (Callee Saved)";
          }
          else if ( i >= 28 && i <= 31 ){
            os << " --- t" << std::dec << i-25 << " (Temporary Register)";
          }
        }
        os << std::endl;
      }
      // Reset the stream to its default state (ie. Not hex)
      os << std::dec << std::left;
      return os;
    }

  private:
    uint32_t ParentThreadID;
    uint64_t StackPtr;                             /// Starting stack pointer for this thread
    uint64_t FirstPC;
    std::shared_ptr<MemSegment> ThreadMem;         /// Pointer to its thread memory (TLS & Stack) 
    
    uint32_t ThreadID;
    uint64_t ThreadPtr;                            /// Thread pointer for this thread
    ThreadState State = ThreadState::START;        /// Thread state (unused)
    RevRegFile RegFile;                            /// Each context has its own register file
    uint64_t ThreadIDAddr = _INVALID_ADDR_;                         /// Used to store the ThreadID
    std::vector<uint32_t> ChildrenThreadIDs = {};  /// List of a thread's children (unused)
    std::vector<int> fildes = {0, 1, 2};           /// Initial fildes are STDOUT, STDIN, and STDERR 

    uint32_t WaitingToJoinTID = __INVALID_TID__;
};


#endif
