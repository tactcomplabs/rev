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
#include <cstdint>
#include <set>
#include <vector>

// -- Rev Headers
#include "RevInstTable.h"
#include "RevMem.h"

namespace SST::RevCPU{

// Enum for tracking the state of a RevThread.
enum class ThreadState {
  START,    // Initial state; resources are allocated.
  RUNNING,  // Thread is assigned and executing on a Proc/HART.
  BLOCKED,  // Waiting for I/O or thread synchronization.
  READY,    // In the ready list; waiting for CPU (not implemented yet).
  DONE,     // Thread has finished; deallocate resources.
};

class RevThread {
 public:
  RevThread(uint32_t inputThreadID, uint32_t inputParentThreadID,
            uint64_t inputStackPtr, uint64_t inputFirstPC,
            std::shared_ptr<RevMem::MemSegment>& inputThreadMem,
            RevFeature* inputFeature);

  // ThreadID operations
  uint32_t GetThreadID() const { return ThreadID; }
  void SetThreadID(const uint32_t NewThreadID) { ThreadID = NewThreadID; }
  uint32_t GetParentThreadID() const { return ParentThreadID; }
  void SetParentThreadID(const uint32_t NewParentThreadID) {
    ParentThreadID = NewParentThreadID;
  }
  uint32_t GetWaitingToJoinTID() const { return WaitingToJoinTID; }
  void SetWaitingToJoinTID(const uint32_t ThreadToWaitOn) {
    WaitingToJoinTID = ThreadToWaitOn;
  }

  // File descriptor operations
  void AddFD(int fd);
  bool RemoveFD(int fd);
  bool FindFD(int fd);
  std::vector<int> GetFildes();

  // Register file operations
  const RevRegFile* GetConstRegFile() const { return &RegFile; }
  RevRegFile* GetRegFile(){ return &RegFile; }
  void SetRegFile(RevRegFile r);

  // RevFeature operations
  RevFeature* GetFeature() const { return Feature; }
  void SetFeature(RevFeature* r);

  // Thread state operations
  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = newState; }

  // Child thread operations
  bool AddChildThreadID(uint32_t pid);
  bool RemoveChildThreadID(uint32_t pid);

  // State checks
  bool isRunning();
  bool isBlocked();
  bool isDone();

  friend std::ostream& operator<<(std::ostream& os, const RevThread& Thread){

    os << "\n";

    auto Feature = Thread.GetFeature();
    auto RegFile= Thread.GetConstRegFile();

    // Calculate total width of the table
    int tableWidth = 6 /*Reg*/ + 7 /*Alias*/ + 16 /*Value*/ + 23 /*Info*/ + 9 /*Separators*/;

    // Print a top border
    os << "|" << std::string(tableWidth-1, '=') << "|" << '\n';
    
    // Print Thread ID
    os << "| Thread " << Thread.GetThreadID() << std::setw(6) <<  std::string(tableWidth-10, ' ') << "|\n";

    // Print the middle border
    os << "|" << std::string(tableWidth-1, '-') << "|" << '\n';

    std::string StateString = "";
    switch (Thread.GetState()){
      case ThreadState::START:
        StateString = "START";
        break;
      case ThreadState::READY:
        StateString = "READY";
        break;
      case ThreadState::RUNNING:
        StateString = "RUNNING";
        break;
      case ThreadState::BLOCKED:
        StateString = "BLOCKED";
        break;
      case ThreadState::DONE:
        StateString = "DONE";
        break;
      default:
        StateString = "UNKNOWN";
        break;
    }
    // Print a nice header
    os << " ==> State: " << StateString << "\n";
    os << " ==> ParentTID: " << Thread.GetParentThreadID() << "\n";
    os << " ==> Blocked by TID: " ;
    if (Thread.GetWaitingToJoinTID() != __INVALID_TID__) {
      os << Thread.GetWaitingToJoinTID();
    } else {
      os << "N/A";
    }
    os << "\n";

    os << '|' << std::string(tableWidth-1, '-') << '|' << '\n';
    // Table header
    os << "| " << std::setw(4) << "Reg" << " | " << std::setw(5) << "Alias" << " | " << std::setw(19) << "Value" << " | " << std::setw(21) << "Info" << " |\n";
    os << "|------|-------|---------------------|-----------------------|\n";

    // Register aliases and descriptions
    static constexpr const char* aliases[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
    static constexpr const char* info[] = {"Zero Register", "Return Address", "Stack Pointer", "Global Pointer", "Thread Pointer", "Temporary Register", "Temporary Register", "Temporary Register", "Callee Saved Register", "Callee Saved Register", "Arg/Return Register", "Arg/Return Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Temporary Register", "Temporary Register", "Temporary Register", "Temporary Register"};

    // Loop over the registers
    for (size_t i = 0; i < _REV_NUM_REGS_; ++i) {
      uint64_t value = RegFile->GetX<uint64_t>(Feature, i);
      
      os << "| " << std::setw(4) << ("x" + std::to_string(i));
      os << " | " << std::setw(5) << aliases[i];
      os << " | " << std::setw(19) << ("0x" + std::to_string(value));
      os << " | " << std::setw(21) << info[i] << " |\n";
    }
    os << "|" << std::string(tableWidth-1, '-') << "|" << '\n';
    return os;
  }

  std::string to_string() const {
    std::ostringstream oss;
    oss << *this;      // << operator is overloaded above
    return oss.str();  // Return the string
  }

 private:
  uint32_t ThreadID;                                   // Thread ID
  uint32_t ParentThreadID;                             // Parent ThreadID
  uint64_t StackPtr;                                   // Initial stack pointer
  uint64_t FirstPC;                                    // Initial PC
  std::shared_ptr<RevMem::MemSegment> ThreadMem;       // TLS and Stack memory
  RevFeature* Feature;                                 // Feature set for this thread
  uint64_t ThreadPtr;                                  // Thread pointer
  ThreadState State = ThreadState::START;              // Thread state (Initializes as )
  RevRegFile RegFile;                                  // Register file
  std::unordered_set<uint32_t> ChildrenThreadIDs = {}; // Child thread IDs
  std::set<int> fildes = {0, 1, 2  };      // Default file descriptors

  uint32_t WaitingToJoinTID = __INVALID_TID__;

  };
};
#endif
