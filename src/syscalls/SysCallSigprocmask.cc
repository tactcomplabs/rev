//
// SysCallSigprocmask.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallSigprocmask.h"

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool SigprocmaskParameters<RiscvArchType>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = tp;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool SigprocmaskParameters<RiscvArchType>::get<sigset_t*>(const size_t parameter_index, sigset_t * & param) {
    if(parameter_index == 1) {
        param = set;
        return true;
    }
    else if(parameter_index == 2) {
        param = oset;
        return true;
    }

    return false;
}

template<>
template<>
void Sigprocmask<Riscv32>::invoke<clock_t>(Sigprocmask<Riscv32>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
    if(parameters.count() == 3) {
        int how;
        sigset_t * set;
        sigset_t * oset;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, how);
        has_values[1] = parameters.get<sigset *>(1, set);
        has_values[2] = parameters.get<sigset *>(2, oset);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = sigprocmask(how, set, oset);
        }
    }
}

template<>
template<>
void Sigprocmask<Riscv64>::invoke<clock_t>(Sigprocmask<Riscv64>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
    if(parameters.count() == 3) {
        int how;
        sigset_t * set;
        sigset_t * oset;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, how);
        has_values[1] = parameters.get<sigset *>(1, set);
        has_values[2] = parameters.get<sigset *>(2, oset);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = sigprocmask(how, set, oset);
        }
    }
}

template<>
template<>
void Sigprocmask<Riscv128>::invoke<clock_t>(Sigprocmask<Riscv128>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
    if(parameters.count() == 3) {
        int how;
        sigset_t * set;
        sigset_t * oset;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, how);
        has_values[1] = parameters.get<sigset *>(1, set);
        has_values[2] = parameters.get<sigset *>(2, oset);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = sigprocmask(how, set, oset);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
