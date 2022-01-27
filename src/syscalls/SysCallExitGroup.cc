//
// SysCallExitGroup.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallExitGroup.h"
#include <algorithm>

#include <unistd.h>
#include <sys/syscall.h>

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
    return ExitGroupSystemCall<IsRiscv32>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, void_t & value, bool & invo_success) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            invo_success = true;
            syscall(SYS_exit_group, status);
        }
    }

    invo_success = false;    
}

template<>
template<>
void ExitGroupSystemCall<true>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitGroupSystemCall<false>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST