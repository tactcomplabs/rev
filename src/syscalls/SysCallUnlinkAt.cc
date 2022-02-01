//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallUnlinkAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void UnlinkAtSystemCall<Riscv32>::invoke<int>(UnlinkAtSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd, flag;
        std::string pth{};
        
        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, pth);
        has_values[2] = parameters.get<int>(2, flag);

        if(has_values[0] & has_values[1] & has_values[2]) {
            success = true;
            value = unlinkat(fd, pth.c_str(), flag);
        }
    }
}

template<>
template<>
void UnlinkAtSystemCall<Riscv64>::invoke<int>(UnlinkAtSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd, flag;
        std::string pth{};
        
        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, pth);
        has_values[2] = parameters.get<int>(2, flag);

        if(has_values[0] & has_values[1] & has_values[2]) {
            success = true;
            value = unlinkat(fd, pth.c_str(), flag);
        }
    }
}

template<>
template<>
void UnlinkAtSystemCall<Riscv128>::invoke<int>(UnlinkAtSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd, flag;
        std::string pth{};
        
        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, pth);
        has_values[2] = parameters.get<int>(2, flag);

        if(has_values[0] & has_values[1] & has_values[2]) {
            success = true;
            value = unlinkat(fd, pth.c_str(), flag);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST