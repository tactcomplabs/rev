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

template<bool IsRiscv32>
typename ExitSystemCall<IsRiscv32>::RiscvModeIntegerType ExitSystemCall<IsRiscv32>::code() {
    return static_cast<ExitSystemCall<IsRiscv32>::RiscvModeIntegerType>(93);
}

template<>
template<>
bool ExitSystemCall<true>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {

    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            exit(status);
        }
    }

    return false;
}

template<>
template<>
bool ExitSystemCall<false>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {

    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            exit(status);
        }
    }

    return false;
}

} /* end namespace RevCPU */ } // end namespace SST