//
// SysCallGetPid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetPid.h"

namespace SST { namespace RevCPU {

static void invoke_impl(pid_t & value) {
    value = getpid();
}

template<>
template<>
void GetPidSystemCall<Riscv32>::invoke(GetPidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    invoke_impl(value);
}

template<>
template<>
void GetPidSystemCall<Riscv64>::invoke(GetPidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    invoke_impl(value);
}

template<>
template<>
void GetPidSystemCall<Riscv128>::invoke(GetPidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    invoke_impl(value);
}

} /* end namespace RevCPU */ } // end namespace SST