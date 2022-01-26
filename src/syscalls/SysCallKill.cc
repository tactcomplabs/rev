//
// KillSystemCall.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "GetPidSystemCall.h"
#include <algorithm>

#include <unistd.h>
#include <sys/syscall.h>

namespace SST { namespace RevCPU {

size_t KillSystemCallParameters::count() {
    return 2UL;
}

template<>
bool KillSystemCallParameters::get<pid_t>(const size_t parameter_index, pid_t& param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }

    return false;
}

template<>
bool KillSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 1) {
        param = sig;
        return true;
    }

    return false;
}

template<bool IsRiscv32>
typename KillSystemCall<IsRiscv32>::RiscvModeIntegerType KillSystemCall<IsRiscv32>::code() {
    return static_cast<KillSystemCall<IsRiscv32>::RiscvModeIntegerType>(129);
}

template<>
template<>
bool KillSystemCall<true>::invoke<pid_t>(SystemCallParameterInterface & parameters, pid_t & value) {

    if(parameters.count() == 2) {
        pid_t pid = -1;
        int sig = -1;

        bool has_values[2] = { false, false };
        has_value[0] = parameters.get<pid_t>(0, pid);
        has_value[1] = parameters.get<int>(0, sig);

        if(has_value[0] && has_value[1] && pid != -1 && sig != -1) {
            value = kill(pid, sig);
            return true;
        }
    }

    return false;
}

template<>
template<>
bool KillSystemCall<false>::invoke<pid_t>(SystemCallParameterInterface & parameters, int & value) {

    if(parameters.count() == 2) {
        pid_t pid = -1;
        int sig = -1;

        bool has_values[2] = { false, false };
        has_value[0] = parameters.get<pid_t>(0, pid);
        has_value[1] = parameters.get<int>(0, sig);

        if(has_value[0] && has_value[1] && pid != -1 && sig != -1) {
            value = kill(pid, sig);
            return true;
        }
    }

    return false;
}

} /* end namespace RevCPU */ } // end namespace SST