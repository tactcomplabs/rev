//
// SysCallFcntl.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallFcntl.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool FcntlParameters<Riscv32>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 1) {
        param = cmd;
        return true;
    }

    return false;
}

template<>
template<>
bool FcntlParameters<Riscv64>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 1) {
        param = cmd;
        return true;
    }

    return false;
}

template<>
template<>
bool FcntlParameters<Riscv128>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 1) {
        param = cmd;
        return true;
    }

    return false;
}

template<>
template<>
void Fcntl<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
     if(parameters.count() == 1) {
        int fildes;
        int cmd;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<int>(1, cmd);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = fcntl(fildes, cmd);
        }
    }
}

template<>
template<>
void Fcntl<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 1) {
        int fildes;
        int cmd;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<int>(1, cmd);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = fcntl(fildes, cmd);
        }
    }
}

template<>
template<>
void Fcntl<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 1) {
        int fildes;
        int cmd;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fildes);
        has_values[1] = parameters.get<int>(1, cmd);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = fcntl(fildes, cmd);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
