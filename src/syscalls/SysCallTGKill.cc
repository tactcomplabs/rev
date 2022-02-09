//
// SysCallTGKill.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallTGKill.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool TgkillParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = tgid;
        return true;
    }
    else if(parameter_index == 1) {
        param = tid;
        return true;
    }
    else if(parameter_index == 2) {
        param = sig;
        return true;
    }

    return false;
}

template<>
template<>
bool TgkillParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = tgid;
        return true;
    }
    else if(parameter_index == 1) {
        param = tid;
        return true;
    }
    else if(parameter_index == 2) {
        param = sig;
        return true;
    }

    return false;
}

template<>
template<>
bool TgkillParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = tgid;
        return true;
    }
    else if(parameter_index == 1) {
        param = tid;
        return true;
    }
    else if(parameter_index == 2) {
        param = sig;
        return true;
    }

    return false;
}

template<>
template<>
void Tgkill<Riscv32>::invoke<int>(Tgkill<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int tgid;
        int tid;
        int sig;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, tgid);
        has_values[1] = parameters.get<int>(1, tid);
        has_values[1] = parameters.get<int>(2, sig);

        if(has_values[0] & has_values[1] & has_values[2] ) {
            success = true;
            value = tgkill(tgid, tid, sig);
        }
    }
}

template<>
template<>
void Tgkill<Riscv64>::invoke<int>(Tgkill<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int tgid;
        int tid;
        int sig;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, tgid);
        has_values[1] = parameters.get<int>(1, tid);
        has_values[1] = parameters.get<int>(2, sig);

        if(has_values[0] & has_values[1] & has_values[2] ) {
            success = true;
            value = tgkill(tgid, tid, sig);
        }
    }
}
template<>
template<>
void Tgkill<Riscv128>::invoke<int>(Tgkill<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int tgid;
        int tid;
        int sig;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, tgid);
        has_values[1] = parameters.get<int>(1, tid);
        has_values[1] = parameters.get<int>(2, sig);

        if(has_values[0] & has_values[1] & has_values[2] ) {
            success = true;
            value = tgkill(tgid, tid, sig);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
