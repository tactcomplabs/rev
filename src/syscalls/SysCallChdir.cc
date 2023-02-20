//
// SysCallChdir.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallChdir.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Chdir<Riscv32>::invoke<int>(RevRegFile const& regFile, RevMem const& mem, RevInst const& inst) {
   std::size_t strlen = 0;
   memory.FindStringTerminal(dt_u32(td_u32(regFile.RV32[inst.rs1],64)), strlen);

   char addr[strlen];
   memory.ReadMem(dt_u32(td_u32(regFile.RV32[inst.rs1],32), sizeof(char)*strlen, &addr);

   regFile.RV32[inst.rd] = chdir( &addr );
   regFile.RV32_PC += inst.instSize;
}

template<>
template<>
void Chdir<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, RevInst const& inst) {
   std::size_t strlen = 0;
   memory.FindStringTerminal(dt_u64(td_u64(regFile.RV64[inst.rs1],64)), strlen);

   char addr[strlen];
   memory.ReadMem(dt_u64(td_u64(regFile.RV64[inst.rs1],64), sizeof(char)*strlen, &addr);

   regFile.RV64[inst.rd] = chdir( &addr );
   regFile.RV64_PC += inst.instSize;
}

template<>
template<>
void Chdir<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, RevInst const& inst) {
}

} /* end namespace RevCPU */ } // end namespace SST
