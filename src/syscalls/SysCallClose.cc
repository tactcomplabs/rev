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
void Close<Riscv32>::invoke<int>(Close<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;            
            value = close(status);
        }
    }
}

template<>
template<>
void Close<Riscv64>::invoke<int>(Close<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;            
            value = close(status);
        }
    }
}

template<>
template<>
void Close<Riscv128>::invoke<int>(Close<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;            
            value = close(status);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST