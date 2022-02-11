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

namespace SST { namespace RevCPU {

template<>
template<>
bool WritevParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 2) {
        param = iovcnt;
        return true;
    }
    return false;
}

template<>
template<>
bool WritevParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 2) {
        param = iovcnt;
        return true;
    }
    return false;
}

template<>
template<>
bool WritevParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 2) {
        param = iovcnt;
        return true;
    }
    return false;
}

template<>
template<>
bool WritevParameters<Riscv32>::get<iovec_t *>(const size_t parameter_index, iovec_t * & param) {
    if(parameter_index == 0) {
        param = iov;
        return true;
    }

    return false;
}

template<>
template<>
bool WritevParameters<Riscv64>::get<iovec_t *>(const size_t parameter_index, iovec_t * & param) {
    if(parameter_index == 0) {
        param = iov;
        return true;
    }

    return false;
}

template<>
template<>
bool WritevParameters<Riscv128>::get<iovec_t *>(const size_t parameter_index, iovec_t * & param) {
    if(parameter_index == 0) {
        param = iov;
        return true;
    }

    return false;
}

template<>
template<>
void Writev<Riscv32>::invoke<ssize_t>(Writev<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
   if(parameters.count() == 3) {

        int fildes;
        iovec_t * iov;
        int iovcnt;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<iovec_t *>(1, iov);
        has_values[2] = parameters.get<int>(2, iovcnt);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = writev(fildes, iov, iovcnt);
        }
    }
}

template<>
template<>
void Writev<Riscv64>::invoke<ssize_t>(Writev<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
   if(parameters.count() == 3) {

        int fildes;
        iovec_t * iov;
        int iovcnt;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<iovec_t *>(1, iov);
        has_values[2] = parameters.get<int>(2, iovcnt);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = writev(fildes, iov, iovcnt);
        }
    }
}

template<>
template<>
void Writev<Riscv128>::invoke<ssize_t>(Writev<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
   if(parameters.count() == 3) {

        int fildes;
        iovec_t * iov;
        int iovcnt;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<iovec_t *>(1, iov);
        has_values[2] = parameters.get<int>(2, iovcnt);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = writev(fildes, iov, iovcnt);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
