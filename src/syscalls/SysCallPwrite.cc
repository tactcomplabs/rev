//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallPwrite.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Pwrite<Riscv32>::invoke<ssize_t>(Pwrite<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {
        int fd;
        void * buf = 0;
        size_t count;
        off_t offset;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);
        has_values[3] = parameters.get<off_t>(3, offset);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = pwrite(fd, buf, count, offset);
        }
    }
}

template<>
template<>
void Pwrite<Riscv64>::invoke<ssize_t>(Pwrite<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {
        int fd;
        void * buf = 0;
        size_t count;
        off_t offset;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);
        has_values[3] = parameters.get<off_t>(3, offset);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = pwrite(fd, buf, count, offset);
        }
    }
}

template<>
template<>
void Pwrite<Riscv128>::invoke<ssize_t>(Pwrite<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {
        int fd;
        void * buf = 0;
        size_t count;
        off_t offset;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);
        has_values[3] = parameters.get<off_t>(3, offset);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = pwrite(fd, buf, count, offset);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST