#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <sched.h>

struct RevClone {
  // ecall (a7 = 120) -> clone
  static const int value = 120; 
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      int (*fnptr)(void*) = nullptr;
      mem.ReadMem(regFile.RV32[10] + sizeof(std::function<int(void*)>), sizeof(std::function<int(void*)>), &fnptr);
      void * stack = nullptr;
      mem.ReadMem(regFile.RV32[11] + sizeof(void*), sizeof(void*), &stack);
      int flags = 0;
      mem.ReadMem(regFile.RV32[12] + sizeof(size_t), sizeof(size_t), &flags);
      void * arg = nullptr;
      mem.ReadMem(regFile.RV32[12] + sizeof(void*), sizeof(void*), &arg);
    
      const int rc = clone(fnptr, stack, flags, arg);
      return rc; 
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      int (*fnptr)(void*) = nullptr;
      mem.ReadMem(regFile.RV64[10] + sizeof(std::function<int(void*)>), sizeof(std::function<int(void*)>), &fnptr);
      void * stack = nullptr;
      mem.ReadMem(regFile.RV64[11] + sizeof(void*), sizeof(void*), &stack);
      int flags = 0;
      mem.ReadMem(regFile.RV64[12] + sizeof(size_t), sizeof(size_t), &flags);
      void * arg = nullptr;
      mem.ReadMem(regFile.RV64[12] + sizeof(void*), sizeof(void*), &arg);
    
      const int rc = clone(fnptr, stack, flags, arg);
      return rc; 
    }
    return -1;
  }
};
