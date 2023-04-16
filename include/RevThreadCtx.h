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

enum class ThreadState {
  Running,
  Ready,
  Waiting,
  Terminated,
  Aborted
};

class RevThreadCtx {
public:
  RevThreadCtx(ThreadState initialState) : state(initialState) {}
  RevThreadCtx(RevThreadCtx &&) = default;
  RevThreadCtx(const RevThreadCtx &) = default;
  RevThreadCtx &operator=(RevThreadCtx &&) = default;
  RevThreadCtx &operator=(const RevThreadCtx &) = default;
  ~RevThreadCtx();

  ThreadState GetPID() const { return state; }
  void SetPID(uint32_t NewPID) { pid = NewPID; }

  ThreadState GetState() const { return state; }
  void SetState(ThreadState newState) { state = newState; }
  bool isRunning(){ return ( state == ThreadState::Running ); }
  bool isReady(){ return (state == ThreadState::Ready); }
  bool isWaiting(){ return (state == ThreadState::Waiting); }
  bool isTerminated(){ return (state == ThreadState::Terminated); }
  bool isAborted(){ return (state == ThreadState::Aborted); }

  ThreadState GetPC() const { return state; }
  void SetPC(uint32_t NewPC) { PC = NewPC; }

private:
  uint32_t pid;
  ThreadState state;
  uint64_t PC;
  
};


