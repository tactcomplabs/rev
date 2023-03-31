#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

#include <unistd.h>

struct RevWrite{
  // ecall (a7 = 64) -> write
  static const int value = 64; 
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
      int fildes;
      mem.ReadMem(regFile.RV32[10] + sizeof(int), sizeof(int), &fildes);
      void * buf = nullptr;
      mem.ReadMem(regFile.RV32[11] + sizeof(void*), sizeof(void*), &buf);
      std::size_t nbyte = 0;
      mem.ReadMem(regFile.RV32[12] + sizeof(size_t), sizeof(size_t), &nbyte);
    
      const int rc = write(fildes, buf, nbyte);
      return rc; 
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      int fildes;
      mem.ReadMem(regFile.RV64[10] + sizeof(int), sizeof(int), &fildes);
      void * buf = nullptr;
      mem.ReadMem(regFile.RV64[11] + sizeof(void*), sizeof(void*), &buf);
      std::size_t nbyte = 0;
      mem.ReadMem(regFile.RV64[12] + sizeof(size_t), sizeof(size_t), &nbyte);
   
      const int rc = write(fildes, buf, nbyte);
      return rc; 
    }
    return -1;
  }
};
