//
// SysCallGettid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGettid.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void GettidSystemCall<Riscv32>::invoke<int>(GettidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = gettid();
}

template<>
template<>
void GettidSystemCall<Riscv64>::invoke<int>(GettidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = gettid();
}

template<>
template<>
void GettidSystemCall<Riscv128>::invoke<int>(GettidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = gettid();
}

} /* end namespace RevCPU */ } // end namespace SST