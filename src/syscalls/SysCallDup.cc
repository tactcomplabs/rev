//
// SysCallDup.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallDup.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool DupParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool DupParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool DupParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
void Dup<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 1) {
        int fildes;

        bool has_values = parameters.get<int>(0, fildes);

        if(has_values) {
            success = true;
            value = dup(fildes);
        }
    }
}

template<>
template<>
void Dup<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 1) {
        int fildes;

        bool has_values = parameters.get<int>(0, fildes);

        if(has_values) {
            success = true;
            value = dup(fildes);
        }
    }
}

template<>
template<>
void Dup<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 1) {
        int fildes;

        bool has_values = parameters.get<int>(0, fildes);

        if(has_values) {
            success = true;
            value = dup(fildes);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST