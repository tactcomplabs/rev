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

template<typename RiscvArchType>
template<>
bool Brk<RiscvArchType>::get(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
void Brk<RiscvArchType>::invoke<void_ptr>(Brk<RiscvArchType>::SystemCallParameterInterfaceType & parameters, void_ptr & value) {
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
