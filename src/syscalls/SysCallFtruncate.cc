//
// SysCallFtruncate.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallFtruncate.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void FtruncateSystemCall<Riscv32>::invoke<int>(FtruncateSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fildes;
        offset_t length;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<offset_t>(1, path);


        if(has_values[0] && has_values[1]) {
            success = true;
            value = ftruncate(fildes, length);
        }
    }
}

template<>
template<>
void FtruncateSystemCall<Riscv64>::invoke<int>(FtruncateSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fildes;
        offset_t length;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<off_t>(1, path);


        if(has_values[0] && has_values[1]) {
            success = true;
            value = ftruncate(fildes, length);
        }
    }
}

template<>
template<>
void FtruncateSystemCall<Riscv128>::invoke<int>(FtruncateSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fildes;
        offset_t length;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<off_t>(1, path);


        if(has_values[0] && has_values[1]) {
            success = true;
            value = ftruncate(fildes, length);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST