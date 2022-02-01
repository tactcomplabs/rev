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

template<>
void ExitGroupSystemCall<Riscv32>::invoke_impl(ExitGroupSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            invoc_success = true;            
            exit_group(SYS_exit_group, status);
        }
    }

    invoc_success = false;    
}

template<>
void ExitGroupSystemCall<Riscv64>::invoke_impl(ExitGroupSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            invoc_success = true;            
            exit_group(SYS_exit_group, status);
        }
    }

    invoc_success = false;    
}

template<>
void ExitGroupSystemCall<Riscv128>::invoke_impl(ExitGroupSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            invoc_success = true;
            exit_group(SYS_exit_group, status);
        }
    }

    invoc_success = false;    
}

template<>
template<>
void ExitGroupSystemCall<Riscv32>::invoke<void_t>(ExitGroupSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitGroupSystemCall<Riscv64>::invoke<void_t>(ExitGroupSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitGroupSystemCall<Riscv128>::invoke<void_t>(ExitGroupSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST