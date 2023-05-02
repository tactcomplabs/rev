#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>

struct RevPwrite{
  // ecall (a7 = 68) -> pwrite
  static const int value = 68; 
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      int fildes = regFile.RV32[10];
      void * buf = nullptr;
      mem.ReadMem(regFile.RV32[11] + sizeof(void*), sizeof(void*), &buf);
      std::size_t nbyte = static_cast<std::size_t>(regFile.RV32[12]);
      off_t offset = static_cast<off_t>(regFile.RV32[12]);
    
      const int rc = pwrite(fildes, buf, nbyte, offset);
      return rc; 
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      int fildes = regFile.RV64[10];
      void * buf = nullptr;
      mem.ReadMem(regFile.RV64[11] + sizeof(void*), sizeof(void*), &buf);
      std::size_t nbyte = static_cast<std::size_t>(regFile.RV64[12]);
      off_t offset = static_cast<off_t>(regFile.RV64[12]);

      const int rc = pwrite(fildes, buf, nbyte, offset);
      return rc; 
    }
    return -1;
  }
};
