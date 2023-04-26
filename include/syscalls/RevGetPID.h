#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include "sys/stat.h"
#include <filesystem>

// -------------------------------------------------------------------
// NOTE: If you are getting garbage data in your call to chdir
//       or any syscall please see ../../test/syscalls/chdir/chdir.c
//       for more info
// -------------------------------------------------------------------

struct RevGetPID{
  // ecall (a7 = 49) -> chdir
  static const int value = 172; 
  
  template<typename RiscvArchType>
  static int ECall(RevProc& Proc) {

    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      const uint32_t pid = Proc.HartToExecActivePID();
      std::cout << "PID: " << pid << std::endl;
      return pid; 
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      const uint32_t pid = Proc.HartToExecActivePID();
      std::cout << "PID: " << pid << std::endl;
      return pid; 
    }
    return -1;
  }
};
