//
// _RevThreadCtx_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//


#include "../include/RevThreadCtx.h"
#include <algorithm>
#include <iostream>

bool RevThreadCtx::AddChildPID(uint32_t pid){
  if( std::find(ChildrenPIDs.begin(), ChildrenPIDs.end(), pid) != ChildrenPIDs.end() ){
    ChildrenPIDs.push_back(pid);
    return true;
  }
  else{
    std::cout << "Child process with id = " << pid << "already exists" << std::endl;
    return false;
  }
}

bool RevThreadCtx::RemoveChildPID(uint32_t pid){
  auto ChildToErase = std::find(ChildrenPIDs.begin(), ChildrenPIDs.end(), pid); 
  if (ChildToErase != ChildrenPIDs.end() ){
    ChildrenPIDs.erase(ChildToErase);
    return true;
  }
  else{
    std::cout << "Child process with id = " << pid << "doesn't exist" << std::endl;
    return false;
  }
}

bool RevThreadCtx::DuplicateRegFile(RevRegFile& regToDup){
  std::cout << "================================================== " << std::endl;
  std::cout << "Duplicating Register File: " << std::endl;
  std::cout << "================================================== " << std::endl;
  std::cout << "ADDRESS OF INCOMING REGISTER FILE = 0x"
            << std::hex << (uint64_t)(&regToDup) << std::dec << std::endl;
  std::cout << "ADDRESS OF NEW REGISTER FILE = 0x"
            << std::hex << (uint64_t)(&RegFile) << std::dec << std::endl;

  for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
    RegFile.RV32[i] = regToDup.RV32[i];
    RegFile.RV64[i] = regToDup.RV64[i];
    RegFile.SPF[i] = regToDup.SPF[i];
    RegFile.DPF[i] = regToDup.DPF[i];

    RegFile.RV32_Scoreboard[i] = regToDup.RV32_Scoreboard[i];
    RegFile.RV64_Scoreboard[i] = regToDup.RV64_Scoreboard[i];
    RegFile.SPF_Scoreboard[i] = regToDup.SPF_Scoreboard[i];
    RegFile.DPF_Scoreboard[i] = regToDup.DPF_Scoreboard[i];
  }

  RegFile.RV64_SSTATUS = regToDup.RV64_SSTATUS;
  RegFile.RV64_SEPC    = regToDup.RV64_SEPC;
  RegFile.RV64_SCAUSE  = 0;
  RegFile.RV64_STVAL   = regToDup.RV64_STVAL;
  RegFile.RV64_STVEC   = regToDup.RV64_STVEC;

  RegFile.RV32_SSTATUS = regToDup.RV32_SSTATUS;
  RegFile.RV32_SEPC    = regToDup.RV32_SEPC;
  RegFile.RV32_SCAUSE  = 0;
  RegFile.RV32_STVAL   = regToDup.RV32_STVAL;
  RegFile.RV32_STVEC   = regToDup.RV32_STVEC;

  RegFile.RV32_PC = regToDup.RV32_PC;
  RegFile.RV64_PC = regToDup.RV64_PC;
  RegFile.FCSR    = regToDup.FCSR;
  RegFile.cost    = regToDup.cost;
  RegFile.trigger = regToDup.trigger;
  RegFile.Entry   = regToDup.Entry;
  return true;

}
