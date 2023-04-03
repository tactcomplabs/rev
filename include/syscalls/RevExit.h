#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <string>


struct RevExit{
  // ecall (a7 = 93) -> exit
  static const int value = 94;

  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst){
    SST::Output *output = new SST::Output("[RevCPU @t]: ", 0, 0, SST::Output::STDOUT);
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      const uint32_t status = regFile.RV64[10];
      const std::string ExitString = "Encountered exit code with status = " + std::to_string(status) + "\n";
      output->fatal(CALL_INFO, -1, ExitString.c_str());
      // TODO: Figure out if this should be returning something or not... _exit returns void 
      exit(status);
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      const uint64_t status = regFile.RV64[10];
      const std::string ExitString = "Encountered exit code with status = " + std::to_string(status) + "\n";
      output->fatal(CALL_INFO, -1, ExitString.c_str());
      exit(status);
      // TODO: Figure out if this should be returning something or not... _exit returns void 
    }
    return -1;
  }
};
