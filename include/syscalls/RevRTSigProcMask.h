#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>



// NOTE: This is not implemented at all... there is a call to ECALL 135 in fork.s 
struct RevRTSigProcMask {
  // ecall (a7 = 2) -> fork
  static const int value = 135;
  
  template<typename RiscvArchType>
  static int ECall(RevProc& Proc) {
    if constexpr( std::is_same<RiscvArchType, Riscv32>::value){
      return 0;
    }
    else if( std::is_same<RiscvArchType, Riscv64>::value ){
      return 0;
    }
  }
};
// struct RevClone {
//   // ecall (a7 = 120) -> clone
//   static const int value = 120; 
//   
//   template<typename RiscvArchType>
//   static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
//     if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
//       int (*fnptr)(void*) = nullptr;
//       mem.ReadMem(regFile.RV32[10] + sizeof(fnptr), sizeof(fnptr), &fnptr);
//       void * stack = nullptr;
//       mem.ReadMem(regFile.RV32[11] + sizeof(void*), sizeof(void*), &stack);
//       int flags = regFile.RV32[12];
//       void * arg = nullptr;
//       mem.ReadMem(regFile.RV32[12] + sizeof(void*), sizeof(void*), &arg);
//     
//       const int rc = clone(fnptr, stack, flags, arg);
//       return rc; 
//     }
//     else if (std::is_same<RiscvArchType, Riscv64>::value){
//       int (*fnptr)(void*) = nullptr;
//       mem.ReadMem(regFile.RV64[10] + sizeof(fnptr), sizeof(fnptr), &fnptr);
//       void * stack = nullptr;
//       mem.ReadMem(regFile.RV64[11] + sizeof(void*), sizeof(void*), &stack);
//       int flags = regFile.RV64[12];
//       void * arg = nullptr;
//       mem.ReadMem(regFile.RV64[12] + sizeof(void*), sizeof(void*), &arg);
//     
//       const int rc = clone(fnptr, stack, flags, arg);
//       return rc; 
//     }
//     return -1;
//   }
// };
