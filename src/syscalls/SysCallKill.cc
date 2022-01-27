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
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

size_t KillSystemCallParameters::count() {
    return 2UL;
}

template<> inline
bool KillSystemCallParameters::get<pid_t>(const size_t parameter_index, pid_t& param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename KillSystemCall<RiscvArchType>::RiscvModeIntegerType KillSystemCall<RiscvArchType>::code() {
    return KillSystemCall<RiscvArchType>::code_value;
}

void invoke_impl(SystemCallParameterInterface & parameters, int & value, bool & invoc_success) {
    if(parameters.count() == 2) {
        pid_t pid = -1;
        int sig = -1;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(0, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            invoc_success = true;
            value = kill(pid, sig);
        }
    }

    invoc_success = false;
}

template<>
template<>
void KillSystemCall<Riscv32>::invoke<int>(SystemCallParameterInterface & parameters, int & value) {
    invoke_impl(parameters, value, success);

}

template<>
template<>
void KillSystemCall<Riscv64>::invoke<int>(SystemCallParameterInterface & parameters, int & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void KillSystemCall<Riscv128>::invoke<int>(SystemCallParameterInterface & parameters, int & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST