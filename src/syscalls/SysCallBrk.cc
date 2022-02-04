//
// SysCallBrk.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallBrk.h"
#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Brk<Riscv32>::invoke<void_ptr>(Brk<Riscv32>::SystemCallParameterInterfaceType & parameters, void_ptr & value) {
    if(parameters.count() == 1) {
        cvoid_ptr addr;
        const bool has_value = parameters.get<cvoid_ptr>(0, addr);
        if(has_value) {
            success = true;
            value = brk(addr);
        }
    }
}

template<>
template<>
void Brk<Riscv64>::invoke<void_ptr>(Brk<Riscv64>::SystemCallParameterInterfaceType & parameters, void_ptr & value) {
    if(parameters.count() == 1) {
        cvoid_ptr addr;
        const bool has_value = parameters.get<cvoid_ptr>(0, addr);
        if(has_value) {
            success = true;
            value = brk(addr);
        }
    }
}

template<>
template<>
void Brk<Riscv128>::invoke<void_ptr>(Brk<Riscv128>::SystemCallParameterInterfaceType & parameters, void_ptr & value) {
    if(parameters.count() == 1) {
        cvoid_ptr addr;
        const bool has_value = parameters.get<cvoid_ptr>(0, addr);
        if(has_value) {
            success = true;
            value = brk(addr);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST