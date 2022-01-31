//
// SysCallExit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallClose.h"
#include <algorithm>

namespace SST { namespace RevCPU {

size_t BrkSystemCallParameters::count() {
    return 1UL;
}

template<>
bool BrkSystemCallParameters::cvoid_ptr<int>(const size_t parameter_index, cvoid_ptr& param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename BrkSystemCall<RiscvArchType>::RiscvModeIntegerType BrkSystemCall<RiscvArchType>::code() {
    return BrkSystemCall<RiscvArchType>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, void_ptr & value) {
    if(parameters.count() == 1) {
        cvoid_ptr addr = -1;
        const bool has_value = parameters.get<cvoid_ptr>(0, addr);
        if(has_value && status != -1) {
            invoc_success = true;            
            value = brk(addr);
        }
    }

    invoc_success = false;    
}

template<>
template<>
void BrkSystemCall<Riscv32>::invoke<void_ptr>(SystemCallParameterInterface & parameters, void_ptr & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST