//
// SysCallMkdirAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMkdirAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void MkdirAtSystemCall<Riscv32>::invoke<int>(SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        std::string buf;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);

        if(has_values[0] && has_values[1] && has_values[1]) {
            success = true;
            value = mkdirat(fd, buf.c_str(), count);
        }
    }
}

template<>
template<>
void MkdirAtSystemCall<Riscv64>::invoke<int>(SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        std::string buf;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);

        if(has_values[0] && has_values[1] && has_values[1]) {
            success = true;
            value = mkdirat(fd, buf.c_str(), count);
        }
    }
}

template<>
template<>
void MkdirAtSystemCall<Riscv128>::invoke<int>(SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        std::string buf;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);

        if(has_values[0] && has_values[1] && has_values[1]) {
            success = true;
            value = mkdirat(fd, buf.c_str(), count);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST