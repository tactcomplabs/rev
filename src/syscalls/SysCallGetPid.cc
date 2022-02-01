//
// SysCallGetpid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetpid.h"

namespace SST { namespace RevCPU {

static void invoke_impl(pid_t & value) {
    value = getpid();
}

template<>
template<>
void GetpidSystemCall<Riscv32>::invoke(GetpidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    invoke_impl(value);
}

template<>
template<>
void GetpidSystemCall<Riscv64>::invoke(GetpidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    invoke_impl(value);
}

template<>
template<>
void GetpidSystemCall<Riscv128>::invoke(GetpidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    invoke_impl(value);
}

} /* end namespace RevCPU */ } // end namespace SST