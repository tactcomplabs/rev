//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallLink.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void LinkSystemCall<Riscv32>::invoke<int>(LinkSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

     if(parameters.count() == 2) {
        std::string oldpth{};
        std::string newpth{};
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, oldpth);
        has_values[1] = parameters.get<std::string>(1, newpth);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = link(oldpth.c_str(), oldpth.c_str());
        }
    }
}

template<>
template<>
void LinkSystemCall<Riscv64>::invoke<int>(LinkSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 2) {
        std::string oldpth{};
        std::string newpth{};
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, oldpth);
        has_values[1] = parameters.get<std::string>(1, newpth);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = link(oldpth.c_str(), oldpth.c_str());
        }
    }
}

template<>
template<>
void LinkSystemCall<Riscv128>::invoke<int>(LinkSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 2) {
        std::string oldpth{};
        std::string newpth{};
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, oldpth);
        has_values[1] = parameters.get<std::string>(1, newpth);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = link(oldpth.c_str(), oldpth.c_str());
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST