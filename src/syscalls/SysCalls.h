
//
// SysCalls.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLS_H__
#define __SYSTEMCALLS_H__

#include "SysCallExit.h"
#include "SysCallExitGroup.h"
#include "SysCallGetPid.h"
#include "SysCallKill.h"
#include "SysCallTGKill.h"

#include <unordered_map>
#include <type_traits>

namespace SST { namespace RevCPU {

template<bool IsRiscv32>
class SystemCalls {

    using RiscvModeIntegerType = typename std::conditional<IsRiscv32, std::uint32_t, std::uint64_t>::type;

    public:

    enum Codes : RiscvModeIntegerType {
        EXIT = ExitSystemCall<IsRiscv32>::code_value,
        EXITGROUP = ExitGroupSystemCall<IsRiscv32>::code_value,
        GETPID = GetPidSystemCall<IsRiscv32>::code_value,
        KILL = KillSystemCall<IsRiscv32>::code_value,
        TGKILL = TGKillSystemCall<IsRiscv32>::code_value,
    };

    static std::unordered_map<Codes, SystemCallInterface<IsRiscv32>> jump_table;
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF