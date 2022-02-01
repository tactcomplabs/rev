//
// SysCallKill.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallKill.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void KillSystemCall<Riscv32>::invoke<int>(KillSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        pid_t pid = -1;
        int sig = -1;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(0, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            success = true;
            value = kill(pid, sig);
        }
    }
}

template<>
template<>
void KillSystemCall<Riscv64>::invoke<int>(KillSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        pid_t pid = -1;
        int sig = -1;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(0, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            success = true;
            value = kill(pid, sig);
        }
    }
}

template<>
template<>
void KillSystemCall<Riscv128>::invoke<int>(KillSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        pid_t pid = -1;
        int sig = -1;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(0, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            success = true;
            value = kill(pid, sig);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST