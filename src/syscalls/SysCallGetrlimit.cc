
//
// SysCallGetrlimit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetrlimit.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool GetrlimitParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = resource;
        return true;
    }

    return false;
}

template<>
template<>
bool GetrlimitParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = resource;
        return true;
    }

    return false;
}

template<>
template<>
bool GetrlimitParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = resource;
        return true;
    }

    return false;
}

template<>
template<>
bool GetrlimitParameters<Riscv32>::get<rlimit*>(const size_t parameter_index, rlimit* & param) {
    if(parameter_index == 1) {
        param = rlp;
        return true;
    }

    return false;
}

template<>
template<>
bool GetrlimitParameters<Riscv64>::get<rlimit*>(const size_t parameter_index, rlimit* & param) {
    if(parameter_index == 1) {
        param = rlp;
        return true;
    }

    return false;
}

template<>
template<>
bool GetrlimitParameters<Riscv128>::get<rlimit*>(const size_t parameter_index, rlimit* & param) {
    if(parameter_index == 1) {
        param = rlp;
        return true;
    }

    return false;
}

template<>
template<>
void Getrlimit<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit *>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = getrlimit(resource, rlp);
        }
    }
}


template<>
template<>
void Getrlimit<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit *>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = getrlimit(resource, rlp);
        }
    }
}

template<>
template<>
void Getrlimit<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit *>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = getrlimit(resource, rlp);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
