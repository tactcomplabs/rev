#pragma once
#ifndef _REV_SYSCALL_INTERFACE
#define _REV_SYSCALL_INTERFACE


#define _SYSCALL_DEBUG_ 1 
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <functional>

#include "RevInstTable.h"

namespace SST { namespace RevCPU {


typedef unsigned int uint128_t __attribute__((mode(TI)));
using cvoid_ptr = const void *;
using cchar_ptr = const char *;
using void_ptr = void *;
using char_ptr = char *;
/*
 * void_t
 *
 * tag-type representing return type for system calls that return void
 */
struct void_t {};
// type tags for each riscv architecture supported in REV
//
struct RiscvArch {
    using int_type = std::uint32_t;
};
struct Riscv32 : RiscvArch {
    using int_type = std::uint32_t;
};
struct Riscv64 : RiscvArch {
    using int_type = std::uint64_t;
};
struct Riscv128 : RiscvArch {
    using int_type = uint128_t;
};
template<typename RiscvArchType>
struct SystemArch {
    
    using RiscvArch = RiscvArchType;
    using IsRiscv32  = typename std::conditional< std::is_same<RiscvArchType, Riscv32>::value, std::true_type, std::false_type>::type;
    using IsRiscv64  = typename std::conditional< std::is_same<RiscvArchType, Riscv64>::value, std::true_type, std::false_type>::type;
    using IsRiscv128 = typename std::conditional< std::is_same<RiscvArchType, Riscv128>::value, std::true_type, std::false_type>::type;
    using RiscvModeIntegerType = typename std::conditional< IsRiscv32::value,
        Riscv32::int_type, // TRUE
        typename std::conditional< IsRiscv64::value, // FALSE
                Riscv32::int_type, // TRUE
                typename std::conditional< IsRiscv128::value, // FALSE
                        Riscv128::int_type, // TRUE
                        Riscv32::int_type // FALSE - we fall back on Riscv32
                >::type
            >::type
        >::type;
};

template<typename RiscvArchType, typename SystemArch<RiscvArchType>::RiscvModeIntegerType Code>
struct SystemCallInterfaceCode {
    using SystemCallCodeType = std::integral_constant<typename SystemArch<RiscvArchType>::RiscvModeIntegerType, static_cast<typename SystemArch<RiscvArchType>::RiscvModeIntegerType>(Code)>;
};


static void DumpRegisters(const uint64_t RegFile[32], const char& RegType){
  std::vector<std::string> Output;
  std::string RegOutStr;
  switch (RegType) {
    case 'a':
      RegOutStr = "--------------- A REGISTERS ---------------";
      Output.push_back(RegOutStr);
      for( unsigned a_reg = 10; a_reg < 18; a_reg++ ){
        RegOutStr = "DBG --- Reg [a" + std::to_string(a_reg-10) + "] = " + std::to_string(RegFile[a_reg]);
        Output.push_back(RegOutStr);
      }
      RegOutStr = "-------------------------------------------";
      Output.push_back(RegOutStr);
    break;
    case 's':
      RegOutStr = "--------------- S REGISTERS ---------------";
      Output.push_back(RegOutStr);
      for( unsigned s_reg = 18; s_reg < 28; s_reg++ ){
        RegOutStr = "DBG --- Reg [s" + std::to_string(s_reg-10) + "] = " + std::to_string(RegFile[s_reg]);
        Output.push_back(RegOutStr);
      }
      RegOutStr = "-------------------------------------------";
      Output.push_back(RegOutStr);
    break;
    // case 't':
    //   RegOutStr = "--------------- T REGISTERS ---------------";
    //   for( unsigned t_reg = 5; t_reg < 8; t_reg++ ){
    //     RegOutStr = "DBG --- Reg [t" + std::to_string(t_reg-10) + "] = " + std::to_string(RegFile[t_reg]);
    //   }
    //   for( unsigned t_reg = 28; t_reg < 32; t_reg++ ){
    //     RegOutStr = "DBG --- Reg [t" + std::to_string(t_reg-10) + "] = " + std::to_string(RegFile[t_reg]);
    //   }
    //   RegOutStr = "-------------------------------------------";
    // break;
    // case 'f':
    //   Output.emplace_back("--------------- F REGIFTERS ---------------");
    //   for( unsigned f_reg = 10; f_reg < 18; f_reg++ ){
    //     Output.emplace_back("DBG --- Reg [f" + std::to_string(s_reg-10) + "] = " + RegFile[s_reg]);
    //   }
    //   Output.emplace_back("-------------------------------------------");
    // break;
    default:
      Output.emplace_back("Please specify 'a', 's', 't'");
      break;
  }

  for( unsigned i = 0; i<Output.size(); i++ ){
    std::cout << Output.at(i) << std::endl;
  }
}

using systemcall_t = std::function<int(RevRegFile &, RevMem &, RevInst &)>;

} /* end namespace RevCPU */ } // end namespace SST
#endif // !_REV_SYSCALL_INTERFACE
