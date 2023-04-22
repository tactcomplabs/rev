#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <string>


struct Rev99{
  // ecall (a7 = 93) -> exit
  static const int value = 99;

  template<typename RiscvArchType>
  static int ECall(RevProc& Proc){
    // RevMem& Mem = Proc.GetMem();
    // RevRegFile& RegFile = Proc.GetHWThreadToExecRegFile();

    SST::Output *output = new SST::Output("[RevCPU @t]: ", 0, 0, SST::Output::STDOUT);
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      // const uint64_t status = RegFile.RV64[10];
       
      /* Check if the active thread is the original */
        return 0;
    }
    return -1;
  }
};
