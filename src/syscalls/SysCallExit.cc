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


template<typename RiscvArchType>
template<>
bool ExitParameters<RiscvArchType>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = status;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
void Exit<RiscvArchType>::invoke<void_t>(Exit<RiscvArchType>::SystemCallParameterInterfaceType & parameters, void_t & value) {
    if(parameters.count() == 1) {
        int status;
        const bool has_value = parameters.get<int>(0, status);
        if(has_value) {
            success = true;            
            exit(status);
        }
    }

    success = false;    
}

} /* end namespace RevCPU */ } // end namespace SST
