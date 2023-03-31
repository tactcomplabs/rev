#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>

struct RevFork {
  // ecall (a7 = 2) -> fork
  static const int value = 2; 
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    pid_t rc = fork();
    return rc;
  }
};