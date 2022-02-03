//
// SysCallDup.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallDup.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void DupSystemCall<Riscv32>::invoke<int>(DupSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int fildes;

        bool has_values = parameters.get<int>(0, fildes);

        if(has_values) {
            success = true;
            value = dup(fildes);
        }
    }
}

template<>
template<>
void DupSystemCall<Riscv64>::invoke<int>(DupSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int fildes;

        bool has_values = parameters.get<int>(0, fildes);

        if(has_values) {
            success = true;
            value = dup(fildes);
        }
    }
}

template<>
template<>
void DupSystemCall<Riscv128>::invoke<int>(DupSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        int fildes;

        bool has_values = parameters.get<int>(0, fildes);

        if(has_values) {
            success = true;
            value = dup(fildes);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST