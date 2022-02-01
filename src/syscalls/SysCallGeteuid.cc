//
// SysCallGetuid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGeteuid.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void GeteuidSystemCall<Riscv32>::invoke<int>(GeteuidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = geteuid();
}

template<>
template<>
void GeteuidSystemCall<Riscv64>::invoke<int>(GeteuidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = geteuid();
}

template<>
template<>
void GeteuidSystemCall<Riscv128>::invoke<int>(GeteuidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = geteuid();
}

} /* end namespace RevCPU */ } // end namespace SST