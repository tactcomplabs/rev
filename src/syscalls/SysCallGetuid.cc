//
// SysCallGetuid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetuid.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void GetuidSystemCall<Riscv32>::invoke<int>(GetuidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getuid();
}

template<>
template<>
void GetuidSystemCall<Riscv64>::invoke<int>(GetuidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getuid();
}

template<>
template<>
void GetuidSystemCall<Riscv128>::invoke<int>(GetuidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getuid();
}

} /* end namespace RevCPU */ } // end namespace SST