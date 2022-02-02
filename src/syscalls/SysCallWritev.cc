//
// SysCallWritev.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallWritev.h"
#include <algorithm>

#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
void WritevSystemCall<Riscv32>::invoke<ssize_t>(WritevSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
   if(parameters.count() == 3) {

        int fildes;
        iovec * iov;
        int iovcnt;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<iovec *>(0, iov);
        has_values[2] = parameters.get<int>(0, iovcnt);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = writev(fildes, iov, iovcnt);
        }
    }
}

template<>
template<>
void WritevSystemCall<Riscv64>::invoke<ssize_t>(WritevSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
   if(parameters.count() == 3) {

        int fildes;
        iovec * iov;
        int iovcnt;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<iovec *>(0, iov);
        has_values[2] = parameters.get<int>(0, iovcnt);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = writev(fildes, iov, iovcnt);
        }
    }
}

template<>
template<>
void WritevSystemCall<Riscv128>::invoke<ssize_t>(WritevSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        int fildes;
        iovec * iov;
        int iovcnt;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<iovec *>(0, iov);
        has_values[2] = parameters.get<int>(0, iovcnt);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = writev(fildes, iov, iovcnt);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST