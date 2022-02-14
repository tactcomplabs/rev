//
// SysCallSetrobustlist.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallSetrobustlist.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

    template<>
    template<>
    bool SetrobustlistParameters<Riscv32>::get<robust_list_head*>(const size_t parameter_index, robust_list_head* & param) {
        if(parameter_index == 0) {
            param = hptr;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool SetrobustlistParameters<Riscv64>::get<robust_list_head*>(const size_t parameter_index, robust_list_head* & param) {
        if(parameter_index == 0) {
            param = hptr;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool SetrobustlistParameters<Riscv128>::get<robust_list_head*>(const size_t parameter_index, robust_list_head* & param) {
        if(parameter_index == 0) {
            param = hptr;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool SetrobustlistParameters<Riscv32>::get<size_t>(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = len;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool SetrobustlistParameters<Riscv64>::get<size_t>(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = len;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool SetrobustlistParameters<Riscv128>::get<size_t>(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = len;
            return true;
        }

        return false;
    }

template<>
template<>
void Setrobustlist<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        robust_list_head * hptr;
        size_t len;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<robust_list_head*>(0, hptr);
        has_values[1] = parameters.get<size_t>(1, len);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = syscall(SYS_set_robust_list, hptr, len);
        }
    }
}

template<>
template<>
void Setrobustlist<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        robust_list_head * hptr;
        size_t len;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<robust_list_head*>(0, hptr);
        has_values[1] = parameters.get<size_t>(1, len);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = syscall(SYS_set_robust_list, hptr, len);
        }
    }
}

template<>
template<>
void Setrobustlist<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        robust_list_head * hptr;
        size_t len;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<robust_list_head*>(0, hptr);
        has_values[1] = parameters.get<size_t>(1, len);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = syscall(SYS_set_robust_list, hptr, len);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
