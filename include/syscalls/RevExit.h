#ifndef _SST_REVCPU_REVEXIT_H_
#define _SST_REVCPU_REVEXIT_H_

#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <string>

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{

struct RevExit{
  // ecall (a7 = 93) -> exit
  static const int value = 94;

  template<typename RiscvArchType>
  static int ECall(RevProc& Proc){
    RevMem& Mem = Proc.GetMem();

    // RevRegFile* RegFile = Proc.GetActiveCtx().RegFile;

    // SST::Output *output = new SST::Output("[RevCPU @t]: ", 0, 0, SST::Output::STDOUT);
    // if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
    //   const uint32_t status = RegFile->RV64[10];
    //   const std::string ExitString = "Encountered exit code with status = " + std::to_string(status) + "\n";
    //   output->fatal(CALL_INFO, -1, ExitString.c_str());
    //   return status;
    // }
    // else if (std::is_same<RiscvArchType, Riscv64>::value){
    //   const uint64_t status = RegFile->RV64[10];

    //   const std::string ExitString = "ECALL: Encountered exit code with status = " + std::to_string(status) + "\n";
    //   output->fatal(CALL_INFO, -1, ExitString.c_str());
    //    
    //   /* Check if the active thread is the original */
    //   return status;
    // }
    return -1;
  }
};

}}

#endif
