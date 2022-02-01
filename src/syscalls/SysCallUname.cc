//
// SysCallUname.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallUname.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void UnameSystemCall<Riscv32>::invoke<int>(UnameSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        utsname * name;
        
        const bool has_values = parameters.get<utsname *>(0, name);
        if(has_values) {
            success = true;
            value = uname(name);
        }
    }
}

template<>
template<>
void UnameSystemCall<Riscv64>::invoke<int>(UnameSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        utsname * name;
        
        const bool has_values = parameters.get<utsname *>(0, name);
        if(has_values) {
            success = true;
            value = uname(name);
        }
    }
}

template<>
template<>
void UnameSystemCall<Riscv128>::invoke<int>(UnameSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        utsname * name;
        
        const bool has_values = parameters.get<utsname *>(0, name);
        if(has_values) {
            success = true;
            value = uname(name);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST