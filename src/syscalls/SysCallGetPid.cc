//
// SysCallGetPid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetPid.h"
#include <algorithm>

#include <unistd.h>

namespace SST { namespace RevCPU {

size_t GetPidSystemCallParameters::count() {
    return 0UL;
}

template<>
bool GetPidSystemCallParameters::get<void_t>(const size_t parameter_index, void_t& param) {
    if(parameter_index == -1) {
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename GetPidSystemCall<RiscvArchType>::RiscvModeIntegerType GetPidSystemCall<RiscvArchType>::code() {
    return GetPidSystemCall<RiscvArchType>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, void_t & value, bool & ivoc_success) {
    if(parameters.count() == 0) {
        ivoc_success = true;
        getpid();
    }

    ivoc_success = false;
}

template<>
template<>
void GetPidSystemCall<Riscv32>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void GetPidSystemCall<Riscv64>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void GetPidSystemCall<Riscv128>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST