//
// SysCallGetdents.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetdents.h"

namespace SST { namespace RevCPU {

template<>
template<>
void Getdents<Riscv32>::invoke<ssize_t>(Getdents<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        int fd;
        void *dirp;
        size_t count;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void *>(1, dirp);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = getdents64(fd, dirp, count);
        }
    }
}

template<>
template<>
void Getdents<Riscv64>::invoke<ssize_t>(Getdents<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        int fd;
        void *dirp;
        size_t count;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void *>(1, dirp);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = getdents64(fd, dirp, count);
        }
    }
}

template<>
template<>
void Getdents<Riscv128>::invoke<ssize_t>(Getdents<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        int fd;
        void *dirp;
        size_t count;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void *>(1, dirp);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = getdents64(fd, dirp, count);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST