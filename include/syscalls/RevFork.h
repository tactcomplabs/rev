#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>

struct RevFork {
  // ecall (a7 = 2) -> fork
  static const int value = 220; 
  
  template<typename RiscvArchType>
  static int ECall(RevProc& Proc) {
    RevMem& Mem = Proc.GetMem();
    RevRegFile& RegFile = Proc.GetHWThreadToExecRegFile();
    std::cout << "INSIDE FORK" << std::endl;
    RevThreadCtx& ParentCtx = Proc.GetActiveCtx();//Proc.GetThreadTable().find(Proc.GetActivePID())->second;

      // .at(Proc.GetActivePID*()];
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      std::cout << "Hello from Software Thread: " << ParentCtx.GetPID() << std::endl;
      
      // TODO: Make a better way of allocating the kids starting mem address (Arg 4)
      uint64_t ChildStartMemAddr = ParentCtx.GetMemStartAddr() + (ParentCtx.GetMemSize() * 2);
      uint32_t NewPID = *std::max_element(Proc.GetPIDs().begin(), Proc.GetPIDs().end())+1;
      std::cout << "The New PID will be: " << NewPID << std::endl;
      if( Proc.CreateCtx(NewPID, ParentCtx.GetPC(),  ParentCtx.GetPID(), ThreadState::Ready,
                                        ChildStartMemAddr, ParentCtx.GetMemSize() != 0 ) ){

        /* Set Parent to Waiting (NOTE: This will change in the future once we support simultaneous multithreading) */
        ParentCtx.SetState(ThreadState::Waiting);
        
        /* Make the child the new active process */
        Proc.SetActivePID(NewPID);
        
        /* Create a copy of Parents Memory Space */
        const char* ParentMem[ParentCtx.GetMemSize()];
        Mem.ReadMem(ParentCtx.GetMemStartAddr(), ParentCtx.GetMemSize(), ParentMem );
        Mem.WriteMem(ChildStartMemAddr, ParentCtx.GetMemSize(), ParentMem);


        // Return 0 to signify we are in the child process
        return 0;
      } else {
        return -1;
      }
    } 
    
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      std::cout << "Hello from Software Thread: " << ParentCtx.GetPID() << std::endl;
      
      // TODO: Make a better way of allocating the kids starting mem address (Arg 4)
      // TODO: Move Mem Calculation Function to RevMem
      uint64_t ChildStartMemAddr = ParentCtx.GetMemStartAddr() + (ParentCtx.GetMemSize() * 2);
      std::cout << "The Child's MemAddr will be: " << std::hex << ChildStartMemAddr << std::endl;
      // TODO: Need to change this to use a global counter
      std::vector<uint32_t> PIDs = Proc.GetPIDs();
      // std::cout << "PIDs = " << PIDs.data() << std::endl;
      for( auto it : PIDs ){
        std::cout << "PID: " << std::to_string(it) << std::endl;
      }
      uint32_t NewPID = ParentCtx.GetPID() + 1;
      std::cout << "The New PID will be: " << NewPID << std::endl;
      if( Proc.CreateCtx(NewPID, ParentCtx.GetPC(),
                         ParentCtx.GetPID(), ThreadState::Ready,
                         ChildStartMemAddr, ParentCtx.GetMemSize()) != 0  ){

        std::cout << "Ctx Created Successfully" << std::endl;

        /* Set Parent to Waiting (NOTE: This will change in the future once we support simultaneous multithreading) */
        ParentCtx.SetState(ThreadState::Waiting);
        
        /* Make the child the new active process */
        Proc.SetActivePID(NewPID);
 
        /* Create a copy of Parents Memory Space */
        const char* ParentMem[ParentCtx.GetMemSize()];
        Mem.ReadMem(ParentCtx.GetMemStartAddr(), ParentCtx.GetMemSize(), ParentMem );
        Mem.WriteMem(ChildStartMemAddr, ParentCtx.GetMemSize(), ParentMem);


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

        RegFile.RV64[10] = ParentCtx.GetPID();
        // RegFile.RV64_PC += Inst.instSize;
        // Proc.GetActiveCtx().GetRegFile().RV64[10] = 0;
        std::cout << "Parent's a0 Register:" << ParentCtx.GetPID() << std::endl;
        std::cout << "New Active PID: " << Proc.GetActivePID() << std::endl;

      // Return 0 to signify we are in the child process
      return 0;
    }
      else {
        return -1;
      }
    }
    return -1;
  }
};
