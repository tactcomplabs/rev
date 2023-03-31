#define _SYSCALL_DEBUG_ 1
#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>

struct RevWrite{

  using value_type = ssize_t;

 // ecall (a7 = 64) -> write
  static const int value = 64; 
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      int fildes = regFile.RV32[10];
      void * buf = nullptr;
      mem.ReadMem(regFile.RV32[11] + sizeof(void*), sizeof(void*), &buf);
      std::size_t nbyte = static_cast<std::size_t>(regFile.RV32[12]);
    
      const int rc = write(fildes, buf, nbyte);
      return rc; 
    }

    else if (std::is_same<RiscvArchType, Riscv64>::value){

      #if _SYSCALL_DEBUG_
      DumpRegisters(regFile.RV64, 'a');
      std::cout << "Executing Write" << std::endl;
      #endif

      int fildes = regFile.RV64[10];

      // mem.ReadMem(regFile.RV64[10], sizeof(int), &fildes);
      std::size_t nbytes = regFile.RV64[12];

      char buf[nbytes];
      char bufchar;

      // const char mystr[3] = "HE";

      // mem.WriteMem(regFile.RV64[11], sizeof(char)*3, (void*)mystr);
      // uint64_t BufAddr = regFile.RV64[12]
      for (unsigned i=0; i<nbytes; i++){
        // mem.ReadU8(regFile.RV64[11]+sizeof(char)*i);//, sizeof(char), &bufchar);
        mem.ReadMem(regFile.RV64[11]+sizeof(char)*i, sizeof(char), &bufchar);
        // std::cout << "BUFFER CHARACTER - " << static_cast<int>(bufchar) << std::endl;
      }
      // std::cout << "S REGISTERS: ";
      // std::cout << regFile.RV64[18] << regFile.RV64[19] << regFile.RV64[20] << std::endl;
      // mem.ReadMem(regFile.RV64[11]+sizeof(char)*i, sizeof(char), &bufchar);

      mem.ReadMem(regFile.RV64[11], sizeof(buf), &buf);
   
      const int rc = write(fildes, buf, nbytes);
      std::cout << "ERROR CODE : " << strerror(errno) << std::endl;
      DumpRegisters(regFile.RV64, 'a');
      return rc; 
    }
    return -1;
  }
};
