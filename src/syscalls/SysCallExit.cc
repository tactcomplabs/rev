//
// SysCallExit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallExit.h"
#include <algorithm>

namespace SST { namespace RevCPU {

size_t ExitSystemCallParameters::count() {
    return 1UL;
}

template<>
bool ExitSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename ExitSystemCall<RiscvArchType>::RiscvModeIntegerType ExitSystemCall<RiscvArchType>::code() {
    return ExitSystemCall<RiscvArchType>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, void_t & value, bool & invoc_success) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            invoc_success = true;            
            exit(status);
        }
    }

    invoc_success = false;    
}

template<>
template<>
void ExitSystemCall<Riscv32>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitSystemCall<Riscv64>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitSystemCall<Riscv128>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST