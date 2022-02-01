//
// SysCallGetgid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetgid.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void GetgidSystemCall<Riscv32>::invoke<int>(GetgidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getgid();
}

template<>
template<>
void GetgidSystemCall<Riscv64>::invoke<int>(GetgidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getgid();
}

template<>
template<>
void GetgidSystemCall<Riscv128>::invoke<int>(GetgidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getgid();
}

} /* end namespace RevCPU */ } // end namespace SST