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

template<>
template<>
void SigactionSystemCall<Riscv32>::invoke<int>(SigactionSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int sig = -1;
        sigaction * act = -1;
        sigaction * oact = -1;

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
void SigactionSystemCall<Riscv64>::invoke<int>(SigactionSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int sig = -1;
        sigaction * act = -1;
        sigaction * oact = -1;

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
void SigactionSystemCall<Riscv128>::invoke<int>(SigactionSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int sig = -1;
        sigaction * act = -1;
        sigaction * oact = -1;

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