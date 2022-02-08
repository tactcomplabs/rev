//
// SysCallSigaction.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallSigaction.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>    
template<>
bool SigactionParameters<RiscvArchType>::get(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = sig;
        return true;
    }

    return false;
}

template<typename RiscvArchType>    
template<>
bool SigactionParameters<RiscvArchType>::get(const size_t parameter_index, sigaction_t * & param) {
    if(parameter_index == 1) {
        param = act;
        return true;
    }
    else if(parameter_index == 2) {
        param = oact;
        return true;
    }

    return false;
}

template<>
template<>
void Sigaction<Riscv32>::invoke<int>(Sigaction<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int sig;
        sigaction * act;
        sigaction * oact;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, sig);
        has_values[1] = parameters.get<sigaction *>(1, act);
        has_values[1] = parameters.get<sigaction *>(2, oact);

        if(has_values[0] & has_values[1] & has_values[2] ) {
            success = true;
            value = sigaction(sig, act, oact);
        }
    }
}

template<>
template<>
void Sigaction<Riscv64>::invoke<int>(Sigaction<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int sig;
        sigaction * act;
        sigaction * oact;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, sig);
        has_values[1] = parameters.get<sigaction *>(1, act);
        has_values[1] = parameters.get<sigaction *>(2, oact);

        if(has_values[0] & has_values[1] & has_values[2] ) {
            success = true;
            value = sigaction(sig, act, oact);
        }
    }
}

template<>
template<>
void Sigaction<Riscv128>::invoke<int>(Sigaction<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int sig;
        sigaction * act;
        sigaction * oact;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, sig);
        has_values[1] = parameters.get<sigaction *>(1, act);
        has_values[1] = parameters.get<sigaction *>(2, oact);

        if(has_values[0] & has_values[1] & has_values[2] ) {
            success = true;
            value = sigaction(sig, act, oact);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
