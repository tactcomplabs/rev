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
#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
void CloseSystemCall<Riscv32>::invoke<int>(CloseSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            success = true;            
            value = close(status);
        }
    }
}

template<>
template<>
void CloseSystemCall<Riscv64>::invoke<int>(CloseSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            success = true;            
            value = close(status);
        }
    }
}

template<>
template<>
void CloseSystemCall<Riscv128>::invoke<int>(CloseSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int status = -1;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value && status != -1) {
            success = true;            
            value = close(status);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST