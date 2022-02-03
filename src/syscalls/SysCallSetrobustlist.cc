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
void SetrobustlistSystemCall<Riscv32>::invoke<int>(SetrobustlistSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        robust_list_head * hptr;
        size_t len;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<robust_list_head*>(0, hptr);
        has_values[1] = parameters.get<size_t>(1, len);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = syscall(SYS_set_robust_list, listhead, len);
        }
    }
}

template<>
template<>
void SetrobustlistSystemCall<Riscv64>::invoke<int>(SetrobustlistSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        robust_list_head * hptr;
        size_t len;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<robust_list_head*>(0, hptr);
        has_values[1] = parameters.get<size_t>(1, len);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = syscall(SYS_set_robust_list, listhead, len);
        }
    }
}

template<>
template<>
void SetrobustlistSystemCall<Riscv128>::invoke<int>(SetrobustlistSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        robust_list_head * hptr;
        size_t len;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<robust_list_head*>(0, hptr);
        has_values[1] = parameters.get<size_t>(1, len);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = syscall(SYS_set_robust_list, listhead, len);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST