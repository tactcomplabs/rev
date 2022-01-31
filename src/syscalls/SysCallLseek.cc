//
// SysCallExit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallLseek.h"
#include <algorithm>

namespace SST { namespace RevCPU {

size_t LseekSystemCallParameters::count() {
    return 3UL;
}

template<>
bool LseekSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<>
bool LseekSystemCallParameters::get<int>(const size_t parameter_index, off_t& param) {
    if(parameter_index == 1) {
        param = offset;
        return true;
    }

    return false;
}

template<>
bool LseekSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 2) {
        param = whence;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename LseekSystemCall<RiscvArchType>::RiscvModeIntegerType LseekSystemCall<RiscvArchType>::code() {
    return LseekSystemCall<RiscvArchType>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, off_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        off_t offset = -1;
        int whence = -1;

        bool has_value [3] = { false, false, false };
        has_value[0] = parameters.get<int>(0, fd);
        has_value[1] = parameters.get<off_t>(1, offset);
        has_value[2] = parameters.get<int>(2, whence);

        if(has_value[0] && has_value[1] && has_value[2]) {
            invoc_success = true;
            value = lseek(fd, offset, whence);
        }
    }

    invoc_success = false;    
}

template<>
template<>
void LseekSystemCall<Riscv32>::invoke<off_t>(SystemCallParameterInterface & parameters, off_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST