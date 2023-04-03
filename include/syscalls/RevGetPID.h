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
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      const pid_t rc = getpid();
      std::cout << "PID: " << rc << std::endl;
      return rc; 
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      const pid_t rc = getpid();
      std::cout << "PID: " << rc << std::endl;
      return rc; 
    }
    return -1;
  }
};
