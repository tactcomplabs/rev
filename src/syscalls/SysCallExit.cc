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
#include <stdlib.h>

namespace SST { namespace RevCPU {

template<>
void ExitSystemCall<Riscv32>::invoke_impl(ExitSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success) {
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
void ExitSystemCall<Riscv64>::invoke_impl(ExitSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success) {
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
void ExitSystemCall<Riscv128>::invoke_impl(ExitSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success) {
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
void ExitSystemCall<Riscv32>::invoke<void_t>(ExitSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitSystemCall<Riscv64>::invoke<void_t>(ExitSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ExitSystemCall<Riscv128>::invoke<void_t>(ExitSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST