#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include "sys/stat.h"
#include <filesystem>

// -------------------------------------------------------------------
// NOTE: If you are getting garbage data in your call to chdir
//       or any syscall please see ../../test/syscalls/chdir/chdir.c
//       for more info
// -------------------------------------------------------------------

struct RevMkdirat{
  // ecall (a7 = 49) -> chdir
  static const int value = 34;
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {


    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){

      const uint32_t fd = regFile.RV32[10];

      std::string path = "";
      unsigned i=0;
      
      // we don't know how long the path string is so read a byte (char)
      // at a time and search for the string terminator character '\0'
      do {
        char dirchar;
        mem.ReadMem(regFile.RV32[10] + sizeof(char)*i, sizeof(char), &dirchar);
        path = path + dirchar;
        i++;
      } while( path.back() != '\0');

      const uint32_t mode = regFile.RV32[12]; // a1 holds the mode (ie. 0777)
      
      // pass to host OS 
      const int rc = mkdirat(fd, path.data(), mode);
      return rc;
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      #if _SYSCALL_DEBUG_
      DumpRegisters(regFile.RV64, 'a');
      #endif
      const uint64_t fd = regFile.RV32[10];

      std::string path = "";
      unsigned i=0;
      
      // we don't know how long the path string is so read a byte (char)
      // at a time and search for the string terminator character '\0'
      do {
        char dirchar;
        mem.ReadMem(regFile.RV32[10] + sizeof(char)*i, sizeof(char), &dirchar);
        path = path + dirchar;
        i++;
      } while( path.back() != '\0');

      const uint32_t mode = regFile.RV32[12]; // a1 holds the mode (ie. 0777)
      // Pass to host OS 
      const int rc = mkdirat(fd, path.data(), mode); // chdir(path.data());
      #if _SYSCALL_DEBUG_
      std::cout << "DBG -- Return Code From Syscall: " << std::strerror(errno) << std::endl;
      #endif
      return rc; 
    }
    return -1;
  }
};
