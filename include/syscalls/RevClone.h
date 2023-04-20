#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>

struct RevClone {
  // ecall (a7 = 2) -> fork
  static const int value = 17; 
  
  template<typename RiscvArchType>
  static int ECall(RevProc& Proc) {
    RevMem& Mem = Proc.GetMem();
    RevRegFile& RegFile = Proc.GetHWThreadToExecRegFile();
    std::cout << "INSIDE FORK" << std::endl;

    RevThreadCtx& ParentCtx = Proc.GetThreadTable().find(Proc.GetActivePID())->second;
      // .at(Proc.GetActivePID*()];
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      std::cout << "Hello from Software Thread: " << ParentCtx.GetPID() << std::endl;
      
      // TODO: Make a better way of allocating the kids starting mem address (Arg 4)
      uint64_t ChildStartMemAddr = ParentCtx.GetMemStartAddr() + (ParentCtx.GetMemSize() * 2);
      uint32_t NewPID = *std::max_element(Proc.GetPIDs().begin(), Proc.GetPIDs().end())+1;
      if( Proc.CreateCtx(NewPID,
                                        ParentCtx.GetPC(),  ParentCtx.GetPID(), ThreadState::Ready,
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
      uint64_t ChildStartMemAddr = ParentCtx.GetMemStartAddr() + (ParentCtx.GetMemSize() * 2);
      uint32_t NewPID = *std::max_element(Proc.GetPIDs().begin(), Proc.GetPIDs().end())+1;
      if( Proc.CreateCtx(NewPID,
                                        ParentCtx.GetPC(),  ParentCtx.GetPID(), ThreadState::Ready,
                                        ChildStartMemAddr, ParentCtx.GetMemSize() != 0 ) ){

        /* Set Parent to Waiting (NOTE: This will change in the future once we support simultaneous multithreading) */
        ParentCtx.SetState(ThreadState::Waiting);
        
        /* Make the child the new active process */
        Proc.SetActivePID(NewPID);
 

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
    }
      else {
        return -1;
      }
    }
    return -1;
  }
};
// struct RevClone {
//   // ecall (a7 = 120) -> clone
//   static const int value = 120; 
//   
//   template<typename RiscvArchType>
//   static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
//     if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
//       int (*fnptr)(void*) = nullptr;
//       mem.ReadMem(regFile.RV32[10] + sizeof(fnptr), sizeof(fnptr), &fnptr);
//       void * stack = nullptr;
//       mem.ReadMem(regFile.RV32[11] + sizeof(void*), sizeof(void*), &stack);
//       int flags = regFile.RV32[12];
//       void * arg = nullptr;
//       mem.ReadMem(regFile.RV32[12] + sizeof(void*), sizeof(void*), &arg);
//     
//       const int rc = clone(fnptr, stack, flags, arg);
//       return rc; 
//     }
//     else if (std::is_same<RiscvArchType, Riscv64>::value){
//       int (*fnptr)(void*) = nullptr;
//       mem.ReadMem(regFile.RV64[10] + sizeof(fnptr), sizeof(fnptr), &fnptr);
//       void * stack = nullptr;
//       mem.ReadMem(regFile.RV64[11] + sizeof(void*), sizeof(void*), &stack);
//       int flags = regFile.RV64[12];
//       void * arg = nullptr;
//       mem.ReadMem(regFile.RV64[12] + sizeof(void*), sizeof(void*), &arg);
//     
//       const int rc = clone(fnptr, stack, flags, arg);
//       return rc; 
//     }
//     return -1;
//   }
// };
