//
// SysCalls.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCalls.h"

#include <unordered_map>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
std::unordered_map<typename SystemCalls<RiscvArchType>::Codes, SystemCallInterface<RiscvArchType>> SystemCalls<RiscvArchType>::jump_table {
        { SystemCalls<RiscvArchType>::Codes::EXIT, ExitSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::EXITGROUP, ExitGroupSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::GETPID, GetPidSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::KILL, KillSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::TGKILL, TGKillSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::READ, ReadSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITE, WriteSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::OPENAT, OpenAtSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::CLOSE, OpenAtSystemCall<IsRiscv32>{} },
        { SystemCalls<RiscvArchType>::Codes::LSEEK, OpenAtSystemCall<IsRiscv32>{} },
};

} /* end namespace RevCPU */ } // end namespace SST

// EOF