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

// -- Rev Headers
#include "../include/RevMem.h"
#include "RevInstTable.h"

/// RevThreadCtx: Enum for tracking state of a software thread (Unused)
enum class ThreadState {
  Start,    // Allocate Resources 
  Running,  // Has the CPU 
  Blocked,  // Waiting for I/O OR synchronization with another thread (mutex, condition variable, sempahore)
  Ready,    // On the ready list (not implemented yet) waiting for CPU availability 
  Done,     // Deallocate resources
};

class RevThreadCtx {

  private:
    uint32_t ThreadID;                             /// Software ThreadID of thread
    uint32_t ParentThreadID;                       /// Parent Ctx's ThreadID
    uint64_t StackPtr;                             /// Starting stack pointer for this thread
    uint64_t ThreadPtr;                            /// Thread pointer for this thread
    
    ThreadState State = ThreadState::Start;        /// Thread state (unused)
    RevRegFile RegFile;                            /// Each context has its own register file
    std::vector<uint32_t> ChildrenThreadIDs = {};  /// List of a thread's children (unused)
    std::vector<int> fildes = {0, 1, 2};           /// Initial fildes are STDOUT, STDIN, and STDERR 

  public:
    // Constructor that takes a RevRegFile object and a uint32_t ParentThreadID
    RevThreadCtx(const uint32_t& inputThreadID, const uint32_t& inputParentThreadID,
                 const uint64_t& inputStackPtr, const uint64_t& inputThreadPtr)
      : ThreadID(inputThreadID), ParentThreadID(inputParentThreadID), 
        StackPtr(inputStackPtr) , ThreadPtr(inputThreadPtr){
      // Create the RegFile for this thread
      RegFile = RevRegFile(); 
      
      // Set the stack pointer 
      RegFile.RV32[2] = (uint32_t)StackPtr;
      RegFile.RV64[2] = StackPtr;

      // Set the thread pointer
      RegFile.RV32[2] = (uint32_t)ThreadPtr;
      RegFile.RV64[4] = ThreadPtr;
    }
      
    void AddFD(int fd);                             /// RevThreadCtx: Add fd to Ctx's fildes 
    bool RemoveFD(int fd);                          /// RevThreadCtx: Remove fd to Ctx's fildes 
    bool FindFD(int fd);                            /// RevThreadCtx: See if Ctx has ownership of fd
    std::vector<int> GetFildes(){ return fildes; }  /// RevThreadCtx: Get list of file descriptors owned by Ctx

    RevRegFile* GetRegFile() { return &RegFile; }   /// RevThreadCtx: Returns pointer to its register file
    void SetRegFile(RevRegFile r) { RegFile = r; }  /// RevThreadCtx: Sets pointer to its register file

    uint32_t GetThreadID() { return ThreadID; }                                  /// RevThreadCtx: Gets Ctx's ThreadID
    void SetThreadID(uint32_t NewThreadID) { ThreadID = NewThreadID; }           /// RevThreadCtx: Sets Ctx's ThreadID

    uint32_t GetParentThreadID() const { return ParentThreadID; }                /// RevThreadCtx: Gets Ctx's Parent's ThreadID
    void SetParentThreadID(uint32_t parent_pid) { ParentThreadID = parent_pid; } /// RevThreadCtx: Gets Ctx's ThreadID

    ThreadState GetState() const { return State; }                     /// RevThreadCtx: Returns the state (ThreadState) of this Ctx
    void SetState(ThreadState newState) { State = newState; }          /// RevThreadCtx: Used to change ThreadState of this Ctx

    bool AddChildThreadID(uint32_t pid);                               /// RevThreadCtx: Adds a child Ctx's pid to this ones children vector
    bool RemoveChildThreadID(uint32_t pid);                            /// RevThreadCtx: Removes a child Ctx's pid to this ones children vector

    bool isRunning(){ return ( State == ThreadState::Running ); }      /// RevThreadCtx: Checks if Ctx's ThreadState is Running
    bool isBlocked(){ return (State == ThreadState::Blocked); }        /// RevThreadCtx: Checks if Ctx's ThreadState is 
    bool isDone(){ return (State == ThreadState::Done); }              /// RevThroneCtx: Checks if Ctx's ThreadState is Done

};


#endif
