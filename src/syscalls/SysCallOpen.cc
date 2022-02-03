//
// SysCallOpen.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallOpen.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>

namespace SST { namespace RevCPU {

template<>
template<>
void OpenSystemCall<Riscv32>::invoke<int>(OpenSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, path);
        has_values[1] = parameters.get<int>(0, oflag);
        has_values[2] = parameters.get<mode_t>(0, mode);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = open(path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void OpenSystemCall<Riscv64>::invoke<int>(OpenSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, path);
        has_values[1] = parameters.get<int>(0, oflag);
        has_values[2] = parameters.get<mode_t>(0, mode);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = open(path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void OpenSystemCall<Riscv128>::invoke<int>(OpenSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, path);
        has_values[1] = parameters.get<int>(0, oflag);
        has_values[2] = parameters.get<mode_t>(0, mode);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = open(path.c_str(), oflag, mode);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST