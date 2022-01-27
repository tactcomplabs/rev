
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

template<bool IsRiscv32>
struct SystemCalls {

    using RiscvModeIntegerType = typename std::conditional<IsRiscv32, std::uint32_t, std::uint64_t>::type;

    enum Codes : RiscvModeIntegerType {
        EXIT = ExitSystemCall<IsRiscv32>::code_value,
        EXITGROUP = ExitGroupSystemCall<IsRiscv32>::code_value,
        GETPID = GetPidSystemCall<IsRiscv32>::code_value,
        KILL = KillSystemCall<IsRiscv32>::code_value,
        TGKILL = TGKillSystemCall<IsRiscv32>::code_value,
    };

};


#endif

// EOF