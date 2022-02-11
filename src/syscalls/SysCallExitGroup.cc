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

#include <linux/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool ExitgroupParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<>
template<>
bool ExitgroupParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<>
template<>
bool ExitgroupParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<>
template<>
void Exitgroup<Riscv32>::invoke<void_t>(Exitgroup<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;
            syscall(SYS_exit_group, status);
        }
    }
}

template<>
template<>
void Exitgroup<Riscv64>::invoke<void_t>(Exitgroup<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;
            syscall(SYS_exit_group, status);
        }
    }
}

template<>
template<>
void Exitgroup<Riscv128>::invoke<void_t>(Exitgroup<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;
            syscall(SYS_exit_group, status);
        }
    }
}


} /* end namespace RevCPU */ } // end namespace SST
