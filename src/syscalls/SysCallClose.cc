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

template<typename RiscvArchType>
template<>
bool Close<RiscvArchType>::get(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
void Close<RiscvArchType>::invoke<int>(Close<RiscvArchType>::SystemCallParameterInterfaceType & parameters, int & value) {
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
