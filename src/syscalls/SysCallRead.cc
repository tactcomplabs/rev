//
// SysCallRead.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallRead.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Read<Riscv32>::invoke<ssize_t>(Read<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        void * buf = 0;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = read(fd, buf, count);
        }
    }
}

template<>
template<>
void Read<Riscv64>::invoke<ssize_t>(Read<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        void * buf = 0;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = read(fd, buf, count);
        }
    }
}

template<>
template<>
void Read<Riscv128>::invoke<ssize_t>(Read<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        void * buf = 0;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = read(fd, buf, count);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST