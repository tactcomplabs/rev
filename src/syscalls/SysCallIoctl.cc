//
// SysCallIoctl.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallIoctl.h"

#include <unistd.h>
#include <sys/ioctl.h>

namespace SST { namespace RevCPU {

template<>
template<>
void IoctlSystemCall<Riscv32>::invoke<int>(IoctlSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.size() == 2) {
        int fildes;
        unsigned long request;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<unsigned long>(request);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = ioctl(fildes, request);
        }
    }
}

template<>
template<>
void IoctlSystemCall<Riscv64>::invoke<int>(IoctlSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.size() == 2) {
        int fildes;
        unsigned long request;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<unsigned long>(request);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = ioctl(fildes, request);
        }
    }
}

template<>
template<>
void IoctlSystemCall<Riscv128>::invoke<int>(IoctlSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.size() == 2) {
        int fildes;
        unsigned long request;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<unsigned long>(request);
        
        if(has_values[0] && has_values[1]) {
            success = true;
            value = ioctl(fildes, request);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST