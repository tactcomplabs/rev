#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>


/* 
 * Fork should
 * - Create the new Ctx
 * - Add the new Ctx to the Table 
 * - ??? Check if there is an available HART??? 
 *       This could be done in ClockTick after ECALL EXCEPTION 
 */

struct RevFork {
  // ecall (a7 = 2) -> fork
  static const int value = 220; 
  
  template<typename RiscvArchType>
  static int ECall(RevProc& Proc) {
    RevThreadCtx& ParentCtx = Proc.HartToExecActiveCtx();
    // RevMem& Mem = Proc.GetMem();
    std::cout << "FORK: Inside Fork with PROC ACTIVE PID = " << Proc.GetActivePID() << std::endl;

      // .at(Proc.GetActivePID*()];
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
    }
   
    else if (std::is_same<RiscvArchType, Riscv64>::value){

      // TODO: Make a better way of allocating the kids starting mem address (Arg 4)
      // TODO: Move Mem Calculation Function to RevMem
      // uint64_t ChildStartMemAddr = ParentCtx.GetMemStartAddr() + (ParentCtx.GetMemSize() * 2);
      // TODO: Need to change this to use a global counter

      RevThreadCtx& ChildCtx = Proc.CreateChildCtx();
      // TODO: Fix this
      ChildCtx.GetRegFile().RV64[10] = 0;

        /* Create a copy of Parents Memory Space */
        // std::cout << "FORK: About to duplicate parent's memory" << std::endl;
        // const char* ParentMem[Proc.ThreadTable.at(ParentPID).GetMemSize()];
        // Mem.ReadMem(ParentCtx.GetMemStartAddr(), Proc.ThreadTable.at(ParentPID).GetMemSize(), ParentMem );
        // Mem.WriteMem(Proc.GetCtx(ChildPID).MemInfoStartAddr, Proc.ThreadTable.at(ParentPID).GetMemSize(), ParentMem);
        // std::cout << "FORK: Finished mem duplication" << std::endl;

        /* Make the child the new active process */
        // Proc.SetActivePID(ChildPID);

        /* 
         * ===========================================================================================
         * Register File
         * ===========================================================================================
         * We need to duplicate the parent's RegFile to to the Childs
         * - NOTE: when we return from this function, the return value will 
         *         be automatically stored in the Proc.RegFile[threadToExec]'s a0 
         *         register. In a traditional fork code this looks like:
         *         
         *         pid_t pid = fork()
         *         if pid < 0: // Error
         *         else if pid = 0: // New Child Process
         *         else: // Parent Process
         *        
         *         In this case, the value of pid is the value thats returned to a0
         *         It follows that 
         *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
         *         - The Parent's a0 register MUST have its PID in it 
         * ===========================================================================================
         */
        // RegFile = ChildCtx.RegFile;
        // Proc.ThreadTable.at(ParentPID).GetRegFile()->RV64[10] = ParentPID;

        /*
          * Alert the Proc there needs to be a Ctx switch
          * Pass the PID that will be switched to once the 
          * current pipeline is executed until completion
        */
      // }
      Proc.CtxSwitchAlert(ChildCtx.GetPID());
      return ParentCtx.GetPID();
    }

    std::cout << "RETURNING -1" << std::endl;
    return -1;
  }
};
