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

template<bool IsRiscv32>
typename GetPidSystemCall<IsRiscv32>::RiscvModeIntegerType GetPidSystemCall<IsRiscv32>::code() {
    return static_cast<GetPidSystemCall<IsRiscv32>::RiscvModeIntegerType>(172);
}

template<>
template<>
bool GetPidSystemCall<true>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {

    if(parameters.count() == 0) {
        getpid();
    }

    return false;
}

template<>
template<>
bool GetPidSystemCall<false>::invoke<void_t>(SystemCallParameterInterface & parameters, void_t & value) {

    if(parameters.count() == 0) {
        getpid();
    }

    return false;
}

} /* end namespace RevCPU */ } // end namespace SST