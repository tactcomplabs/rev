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

#include <sys/syscall.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool Exitgroup<RiscvArchType>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
void Exitgroup<RiscvArchType>::invoke<void_t>(Exitgroup<RiscvArchType>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;
            exit_group(SYS_exit_group, status);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
