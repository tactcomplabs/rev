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
void Geteuid<Riscv32>::invoke<int>(Geteuid<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = geteuid();
}

template<>
template<>
void Geteuid<Riscv64>::invoke<int>(Geteuid<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = geteuid();
}

template<>
template<>
void Geteuid<Riscv128>::invoke<int>(Geteuid<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = geteuid();
}

} /* end namespace RevCPU */ } // end namespace SST