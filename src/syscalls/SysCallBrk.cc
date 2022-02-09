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
bool BrkParameters<Riscv32>::get<void*>(const size_t parameter_index, void* & param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<>
template<>
bool BrkParameters<Riscv64>::get<void*>(const size_t parameter_index, void* & param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<>
template<>
bool BrkParameters<Riscv128>::get<void*>(const size_t parameter_index, void* & param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<>
template<>
void Brk<Riscv32>::invoke<int>(Brk<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        void* addr;
        const bool has_value = parameters.get<void*>(0, addr);
        if(has_value) {
            success = true;
            value = brk(addr);
        }
    }
}

template<>
template<>
void Brk<Riscv64>::invoke<int>(Brk<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        void* addr;
        const bool has_value = parameters.get<void*>(0, addr);
        if(has_value) {
            success = true;
            value = brk(addr);
        }
    }
}

template<>
template<>
void Brk<Riscv128>::invoke<int>(Brk<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        void* addr;
        const bool has_value = parameters.get<void*>(0, addr);
        if(has_value) {
            success = true;
            value = brk(addr);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
