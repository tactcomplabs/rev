#include "../RevSysCalls.h"
#include "../RevSysCallInterface.h"
#include <filesystem>

// -------------------------------------------------------------------
// NOTE: If you are getting garbage data in your call to chdir
//       or any syscall please see ../../test/syscalls/chdir/chdir.c
//       for more info
// -------------------------------------------------------------------

struct RevChdir{
  // ecall (a7 = 49) -> chdir
  static const int value = 49; 
  
  
  template<typename RiscvArchType>
  static int ECall(RevRegFile& regFile, RevMem& mem, RevInst& inst) {
    if constexpr (std::is_same<RiscvArchType, Riscv32>::value){
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
      
      const int rc = chdir(path.data());
      // Pass to host OS 
      return rc; 
    }
    else if (std::is_same<RiscvArchType, Riscv64>::value){
      std::string path = "";
      unsigned i=0;
      
      // we don't know how long the path string is so read a byte (char)
      // at a time and search for the string terminator character '\0'
      do {
        char dirchar;
        mem.ReadMem(regFile.RV64[10] + sizeof(char)*i, sizeof(char), &dirchar);
        path = path + dirchar;
        i++;
      } while( path.back() != '\0');

      const int rc = chdir(path.data());
      
      return rc; 
    }
    return -1;
  }
};
