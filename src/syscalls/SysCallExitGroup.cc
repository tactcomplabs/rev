//
// SysCallExit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallExitGroup.h"
#include <algorithm>

#include <sys/syscall.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

size_t ExitGroupSystemCallParameters::count() {
    return 1UL;
}

template<>
bool ExitGroupSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<bool IsRiscv32>
typename ExitGroupSystemCall<IsRiscv32>::RiscvModeIntegerType ExitGroupSystemCall<IsRiscv32>::code() {
    return static_cast<ExitGroupSystemCall<IsRiscv32>::RiscvModeIntegerType>(94);
}

template<>
template<>
bool ExitGroupSystemCall<true>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {

    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            syscall(SYS_exit_group, status);
        }
    }

    return false;
}

template<>
template<>
bool ExitGroupSystemCall<false>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {

    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            syscall(SYS_exit_group, status);
        }
    }

    return false;
}

} /* end namespace RevCPU */ } // end namespace SST