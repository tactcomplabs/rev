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

size_t CloseSystemCallParameters::count() {
    return 1UL;
}

template<>
bool CloseSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename CloseSystemCall<RiscvArchType>::RiscvModeIntegerType CloseSystemCall<RiscvArchType>::code() {
    return CloseSystemCall<RiscvArchType>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, int & value) {
    if(parameters.count() == 1) {
        int fd = -1;
        const bool has_value = parameters.get<int>(0, fd);
        if(has_value && status != -1) {
            invoc_success = true;            
            value = close(fd);
        }
    }

    invoc_success = false;    
}

template<>
template<>
void CloseSystemCall<Riscv32>::invoke<int>(SystemCallParameterInterface & parameters, int & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST