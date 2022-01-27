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

template<bool IsRiscv32>
std::unordered_map<typename SystemCalls<IsRiscv32>::Codes, SystemCallInterface<IsRiscv32>> SystemCalls<IsRiscv32>::jump_table {
        { SystemCalls<IsRiscv32>::Codes::EXIT, ExitSystemCall<IsRiscv32>{} },
        { SystemCalls<IsRiscv32>::Codes::EXITGROUP, ExitGroupSystemCall<IsRiscv32>{} },
        { SystemCalls<IsRiscv32>::Codes::GETPID, GetPidSystemCall<IsRiscv32>{} },
        { SystemCalls<IsRiscv32>::Codes::KILL, KillSystemCall<IsRiscv32>{} },
        { SystemCalls<IsRiscv32>::Codes::TGKILL, TGKillSystemCall<IsRiscv32>{} },
};

} /* end namespace RevCPU */ } // end namespace SST

// EOF