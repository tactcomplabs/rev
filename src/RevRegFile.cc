//
// _RevRegFile_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#include "RevRegFile.h"

namespace SST::RevCPU{

// Overload the printing
std::ostream& operator<<(std::ostream& os, const RevRegFile& regFile){

  // Register aliases and descriptions
  static constexpr const char* aliases[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
  static constexpr const char* info[] = {"Zero Register", "Return Address", "Stack Pointer", "Global Pointer", "Thread Pointer", "Temporary Register", "Temporary Register", "Temporary Register", "Callee Saved Register", "Callee Saved Register", "Arg/Return Register", "Arg/Return Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Argument Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Callee Saved Register", "Temporary Register", "Temporary Register", "Temporary Register", "Temporary Register"};

  // Update table width to accommodate the new "Dep" column
  constexpr int tableWidth = 6 /*Reg*/ + 7 /*Alias*/ + 19 /*Value*/ + 6 /*Dep*/ + 23 /*Info*/ + 11 /*Separators*/;


  os << '|' << std::string(tableWidth-3, '-') << '|' << '\n';
  
  // Table header
  os << "| " << std::setw(4) << "Reg" << " | " << std::setw(5) << "Alias" << " | " << std::setw(21) << "Value" << " | " << std::setw(4) << "Dep" << " | " << std::setw(21) << "Info" << " |\n";
  os << "|------|-------|-----------------------|------|-----------------------|\n";

  // Loop over the registers
  for (size_t i = 0; i < _REV_NUM_REGS_; ++i) {
    uint64_t value = regFile.GetX<uint64_t>(i);

    // if scoreboard is not 0, there is a dependency
    char depValue = regFile.RV_Scoreboard[i] ? 'T' : 'F';
    os << "| " << std::setw(4) << ("x" + std::to_string(i));
    os << " | " << std::setw(5) << aliases[i];
    os << " | " << std::setw(21) << ("0x" + std::to_string(value));
    os << " | " << std::setw(4) << depValue;  // New "Dep" column
    os << " | " << std::setw(21) << info[i] << " |\n";
  }
  os << "|" << std::string(tableWidth-3, '-') << "|" << '\n';


  return os;
}

} // namespace SST::RevCPU
