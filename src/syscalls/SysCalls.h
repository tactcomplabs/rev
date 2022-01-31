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

#include "SystemCallInterface.h"
#include "SysCallExit.h"
#include "SysCallExitGroup.h"
#include "SysCallGetPid.h"
#include "SysCallKill.h"
#include "SysCallTGKill.h"
#include "SysCallRead.h"
#include "SysCallWrite.h"

#include <unordered_map>
#include <type_traits>

// https://github.com/riscv-software-src/riscv-pk/blob/master/pk/syscall.h
//
// https://github.com/riscv-software-src/riscv-isa-sim/tree/master/fesvr
//

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
class SystemCalls {

    using IsRiscv32 = typename std::conditional< std::is_same<RiscvArchType, Riscv32>::value, std::true_type, std::false_type>::type;
    using IsRiscv64 = typename std::conditional< std::is_same<RiscvArchType, Riscv64>::value, std::true_type, std::false_type>::type;
    using IsRiscv128 = typename std::conditional< std::is_same<RiscvArchType, Riscv128>::value, std::true_type, std::false_type>::type;
    
    using RiscvModeIntegerType = typename std::conditional< IsRiscv32::value,
        Riscv32::int_type, // TRUE
        typename std::conditional< IsRiscv64::value, // FALSE
                Riscv32::int_type, // TRUE
                typename std::conditional< IsRiscv128::value, // FALSE
                        Riscv128::int_type, // TRUE
                        Riscv32::int_type
                >::type
            >::type
        >::type;

    public:

    enum Codes : RiscvModeIntegerType {
        EXIT = ExitSystemCall<RiscvArchType>::code_value,
        EXITGROUP = ExitGroupSystemCall<RiscvArchType>::code_value,
        GETPID = GetPidSystemCall<RiscvArchType>::code_value,
        KILL = KillSystemCall<RiscvArchType>::code_value,
        TGKILL = TGKillSystemCall<RiscvArchType>::code_value,
        READ = ReadSystemCall<RiscvArchType>::code_value,
        WRITE = WriteSystemCall<RiscvArchType>::code_value,
        OPENAT = OpenAtSystemCall<RiscvArchType>::code_value,
        CLOSE = OpenAtSystemCall<RiscvArchType>::code_value,
    };

    static std::unordered_map<Codes, SystemCallInterface<RiscvArchType>> jump_table;
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF