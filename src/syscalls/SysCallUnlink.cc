//
// SysCallUnlink.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallUnlink.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool Unlink<RiscvArchType>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
void Unlink<RiscvArchType>::invoke<int>(Unlink<RiscvArchType>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        std::string pth{};
        
        const bool has_values = parameters.get<std::string>(0, pth);
        if(has_values) {
            success = true;
            value = unlink(pth.c_str());
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
