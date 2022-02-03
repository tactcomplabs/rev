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

template<>
template<>
void UnlinkSystemCall<Riscv32>::invoke<int>(UnlinkSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        std::string pth{};
        
        const bool has_values = parameters.get<std::string>(0, pth);
        if(has_values) {
            success = true;
            value = unlink(pth.c_str());
        }
    }
}

template<>
template<>
void UnlinkSystemCall<Riscv64>::invoke<int>(UnlinkSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        std::string pth{};
        
        const bool has_values = parameters.get<std::string>(0, pth);
        if(has_values) {
            success = true;
            value = unlink(pth.c_str());
        }
    }
}

template<>
template<>
void UnlinkSystemCall<Riscv128>::invoke<int>(UnlinkSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
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