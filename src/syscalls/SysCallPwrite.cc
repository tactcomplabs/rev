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
void PwriteSystemCall<Riscv32>::invoke<ssize_t>(PwriteSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() >= 4) {
        int fd;
        void * buf = 0;
        size_t count;
        offset_t offset;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);
        has_values[3] = parameters.get<offset_t>(0, offset);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = pwrite(fd, buf, count, offset);
        }
    }
}

template<>
template<>
void PwriteSystemCall<Riscv64>::invoke<ssize_t>(PwriteSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() >= 4) {
        int fd;
        void * buf = 0;
        size_t count;
        offset_t offset;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);
        has_values[3] = parameters.get<offset_t>(0, offset);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = pwrite(fd, buf, count, offset);
        }
    }
}

template<>
template<>
void PwriteSystemCall<Riscv128>::invoke<ssize_t>(PwriteSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() >= 4) {
        int fd;
        void * buf = 0;
        size_t count;
        offset_t offset;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);
        has_values[3] = parameters.get<offset_t>(0, offset);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = pwrite(fd, buf, count, offset);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST