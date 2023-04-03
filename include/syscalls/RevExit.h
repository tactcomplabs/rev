
// John's Message 
// output->fatal(CALL_INFO, -1, "Encountered exit code\n");

#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"


struct RevExit{
  // ecall (a7 = 93) -> exit
  static const int value = 94;

  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst){
    SST::Output *output = new SST::Output("[RevCPU @t]: ", 0, 0, SST::Output::STDOUT);
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      output->fatal(CALL_INFO, -1, "Encountered exit code\n");
      std::cout << "Encountered Exit Code" << std::endl;
      const uint64_t status = regFile.RV64[10];
      exit(status);
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      output->fatal(CALL_INFO, -1, "Encountered exit code\n");
      std::cout << "Encountered Exit Code" << std::endl;
      const uint64_t status = regFile.RV64[10];
      exit(status);
      return 0;
    }
    return -1;
  }
};
