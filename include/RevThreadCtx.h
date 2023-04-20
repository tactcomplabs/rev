/***
ThreadCtx 
- PID
- Process State (running, ready, waiting, terminated)
- Program Counter
- MemInfo (Address & Size)
- TODO: Scheduling Info
- File Descriptors: A list of open files and associated metadata relevant to the process
- I/O status information: Details about the I/O devices being used by the process
- IPC Info: Data related to communication between processes such as message queues, pipes, shared mem
- ParentPID? 

------
Methods to Implement (RevCPU):
- AddCtx(Ctx, bool clone_vm)
- GetCtx(pid)
- RetireCtx(pid)
- SaveRegFiles(RevRegFile)
- GetThreadState(pid)
- PauseThread(pid)
- ReadyThread(pid)
...

Memory Stuff? 
- RelinquishMem
- ShareMem
***/

#include <cstdint>
#include <vector>
#include "../include/RevMem.h"
#include "RevInstTable.h"

enum class ThreadState {
  Running,
  Ready,  
  Waiting,
  Terminated,
  Aborted,
  Retired // TODO: Should this be a state
};

class RevThreadCtx {
public:
  RevThreadCtx(uint32_t pid, uint64_t pc, 
               uint32_t parentPID, ThreadState initialState, 
               uint64_t memInfoStartAddr, uint64_t memInfoSize,
               RevRegFile regFile); // : State(initialState) {}
  // RevThreadCtx(RevThreadCtx &&) = default;
  // RevThreadCtx(const RevThreadCtx &) = default;
  // RevThreadCtx &operator=(RevThreadCtx &&) = default;
  // RevThreadCtx &operator=(const RevThreadCtx &) = default;
  ~RevThreadCtx();

  uint32_t GetPID() const { return PID; }
  void SetPID(uint32_t NewPID) { PID = NewPID; }

  uint32_t GetParentPID() const { return ParentPID; }
  void SetParentPID(uint32_t pid) { ParentPID = pid; }

  ThreadState GetState() const { return State; }
  void SetState(ThreadState newState) { State = newState; }

  RevRegFile& GetRegFile() { return RegFile; }

  uint64_t GetMemStartAddr() const { return MemInfoStartAddr; }
  void SetMemStartAddr(uint64_t newMemStartAddr) { MemInfoStartAddr = newMemStartAddr; }

  uint64_t GetMemSize() const { return MemInfoSize; }
  void SetMemSize(uint64_t newMemSize) { MemInfoSize = newMemSize; }

  bool AddChildPID(uint32_t pid);
  bool RemoveChildPID(uint32_t pid);

  bool isRunning(){ return ( State == ThreadState::Running ); }
  bool isReady(){ return (State == ThreadState::Ready); }
  bool isWaiting(){ return (State == ThreadState::Waiting); }
  bool isTerminated(){ return (State == ThreadState::Terminated); }
  bool isAborted(){ return (State == ThreadState::Aborted); }

  uint64_t GetPC() const { return PC; }
  void SetPC(uint32_t NewPC) { PC = NewPC; }

  bool HasParent(){ return hasParent; }

private:
  uint32_t PID;
  std::vector<uint32_t> ChildrenPIDs;
  uint32_t ParentPID;
  uint64_t MemInfoStartAddr;
  uint64_t MemInfoSize;
  ThreadState State;
  RevRegFile RegFile;
  bool hasParent = false; 

  uint64_t PC;
  
};


