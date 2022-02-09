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

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool Unlinkat<RiscvArchType>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = dirfd;
        return true;
    }
    else if(parameter_index == 2) {
        param = flags;
        return true;
    }
    return false;
}

template<typename RiscvArchType>
template<>
bool Unlinkat<RiscvArchType>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
void Unlinkat<RiscvArchType>::invoke<int>(Unlinkat<RiscvArchType>::SystemCallParameterInterfaceType & parameters, int & value) {
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
