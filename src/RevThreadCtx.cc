#include "../include/RevThreadCtx.h"

RevThreadCtx::RevThreadCtx(uint32_t pid, uint64_t pc, 
               uint32_t parentPID, ThreadState initialState, 
               uint64_t startAddr, uint64_t memSize)
: PID(pid), PC(pc), ParentPID(parentPID), State(ThreadState::Ready),
  MemInfoStartAddr(startAddr), MemInfoSize(memSize) {}


RevThreadCtx::~RevThreadCtx() {
}


