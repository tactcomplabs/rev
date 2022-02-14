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
bool KillParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }
    else if(parameter_index == 1) {
        param = sig;
        return true;
    }

    return false;
}

template<>
template<>
bool KillParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }
    else if(parameter_index == 1) {
        param = sig;
        return true;
    }

    return false;
}

template<>
template<>
bool KillParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }
    else if(parameter_index == 1) {
        param = sig;
        return true;
    }

    return false;
}

template<>
template<>
void Kill<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {
        pid_t pid;
        int sig;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(1, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            success = true;
            value = kill(pid, sig);
        }
    }
}

template<>
template<>
void Kill<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {
        pid_t pid;
        int sig;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(1, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            success = true;
            value = kill(pid, sig);
        }
    }
}

template<>
template<>
void Kill<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {
        pid_t pid;
        int sig;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<pid_t>(0, pid);
        has_values[1] = parameters.get<int>(1, sig);

        if(has_values[0] && has_values[1] && pid != -1 && sig != -1) {
            success = true;
            value = kill(pid, sig);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
