//
// SysCallOpenAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallOpenAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>

namespace SST { namespace RevCPU {

template<>
template<>
void OpenAtSystemCall<Riscv32>::invoke<int>(OpenAtSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 4) {
        int fd;
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(0, path);
        has_values[2] = parameters.get<int>(0, oflag);
        has_values[3] = parameters.get<mode_t>(0, mode);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = openat(fd, path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void OpenAtSystemCall<Riscv64>::invoke<int>(OpenAtSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 4) {
        int fd;
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(0, path);
        has_values[2] = parameters.get<int>(0, oflag);
        has_values[3] = parameters.get<mode_t>(0, mode);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = openat(fd, path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void OpenAtSystemCall<Riscv128>::invoke<int>(OpenAtSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 4) {
        int fd;
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(0, path);
        has_values[2] = parameters.get<int>(0, oflag);
        has_values[3] = parameters.get<mode_t>(0, mode);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = openat(fd, path.c_str(), oflag, mode);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST