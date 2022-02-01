//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallLinkAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void LinkAtSystemCall<Riscv32>::invoke<int>(LinkAtSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 5) {
        std::string oldpth{};
        std::string newpth{};
        int fd1, fd2, flag;
        
        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd1);
        has_values[1] = parameters.get<std::string>(1, oldpth);
        has_values[2] = parameters.get<int>(2, fd2);
        has_values[3] = parameters.get<std::string>(3, newpth);
        has_values[4] = parameters.get<int>(4, flag);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = linkat(fd1, oldpth.c_str(), fd2, oldpth.c_str(), flag);
        }
    }
}

template<>
template<>
void LinkAtSystemCall<Riscv64>::invoke<int>(LinkAtSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 5) {
        std::string oldpth{};
        std::string newpth{};
        int fd1, fd2, flag;
        
        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd1);
        has_values[1] = parameters.get<std::string>(1, oldpth);
        has_values[2] = parameters.get<int>(2, fd2);
        has_values[3] = parameters.get<std::string>(3, newpth);
        has_values[4] = parameters.get<int>(4, flag);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = linkat(fd1, oldpth.c_str(), fd2, oldpth.c_str(), flag);
        }
    }
}

template<>
template<>
void LinkAtSystemCall<Riscv128>::invoke<int>(LinkAtSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 5) {
        std::string oldpth{};
        std::string newpth{};
        int fd1, fd2, flag;
        
        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd1);
        has_values[1] = parameters.get<std::string>(1, oldpth);
        has_values[2] = parameters.get<int>(2, fd2);
        has_values[3] = parameters.get<std::string>(3, newpth);
        has_values[4] = parameters.get<int>(4, flag);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = linkat(fd1, oldpth.c_str(), fd2, oldpth.c_str(), flag);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST