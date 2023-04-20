#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <string>


struct RevExit{
  // ecall (a7 = 93) -> exit
  static const int value = 94;

  template<typename RiscvArchType>
  static int ECall(RevProc& Proc){
    RevMem& Mem = Proc.GetMem();
    RevRegFile& RegFile = Proc.GetHWThreadToExecRegFile();

    SST::Output *output = new SST::Output("[RevCPU @t]: ", 0, 0, SST::Output::STDOUT);
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      const uint32_t status = RegFile.RV64[10];
      const std::string ExitString = "Encountered exit code with status = " + std::to_string(status) + "\n";
      output->fatal(CALL_INFO, -1, ExitString.c_str());
      // TODO: Figure out if this should be returning something or not... _exit returns void 
      exit(status);
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      const uint64_t status = RegFile.RV64[10];
      const std::string ExitString = "ECALL: Encountered exit code with status = " + std::to_string(status) + "\n";
       
      /* Check if the active thread is the original */
      if( Proc.GetActiveCtx().GetParentPID() == 0 ){
        std::cout << "ORIGIN PROCESS FOUND... EXITING" << std::endl;
        output->fatal(CALL_INFO, -1, ExitString.c_str());
        exit(status);
      } else {
        std::cout << "Retiring software thread: " <<  Proc.GetActivePID() << " and transitioning control to parent"<< std::endl;
        Proc.GetActiveCtx().SetState(ThreadState::Retired);
        Proc.SetActivePID(Proc.GetActiveCtx().GetParentPID());
        RevThreadCtx& ParentCtx = Proc.GetActiveCtx();
        RegFile = ParentCtx.GetRegFile();
        std::cout << "New active software thread: " <<  Proc.GetActivePID() << std::endl;
        /* FIXME: Not sure what this should return */
        return 1;
      }
    }
    return -1;
  }
};
