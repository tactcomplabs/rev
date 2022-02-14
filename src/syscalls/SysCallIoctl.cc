//
// SysCallIoctl.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallIoctl.h"

#include <unistd.h>
#include <sys/ioctl.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool IoctlParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool IoctlParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool IoctlParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool IoctlParameters<Riscv32>::get<unsigned long>(const size_t parameter_index, unsigned long& param) {
    if(parameter_index == 1) {
        param = request;
        return true;
    }

    return false;
}

template<>
template<>
bool IoctlParameters<Riscv64>::get<unsigned long>(const size_t parameter_index, unsigned long& param) {
    if(parameter_index == 1) {
        param = request;
        return true;
    }

    return false;
}

template<>
template<>
bool IoctlParameters<Riscv128>::get<unsigned long>(const size_t parameter_index, unsigned long& param) {
    if(parameter_index == 1) {
        param = request;
        return true;
    }

    return false;
}
    
template<>
template<>
void Ioctl<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 2) {
        int fildes;
        unsigned long request;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<unsigned long>(1, request);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = ioctl(fildes, request);
        }
    }
}

template<>
template<>
void Ioctl<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 2) {
        int fildes;
        unsigned long request;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<unsigned long>(1, request);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = ioctl(fildes, request);
        }
    }
}

template<>
template<>
void Ioctl<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 2) {
        int fildes;
        unsigned long request;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<unsigned long>(1, request);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = ioctl(fildes, request);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
