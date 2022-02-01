//
// SysCallGetegid.cc
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
void GetegidSystemCall<Riscv32>::invoke<int>(GetegidSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getegid();
}

template<>
template<>
void GetegidSystemCall<Riscv64>::invoke<int>(GetegidSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getegid();
}

template<>
template<>
void GetegidSystemCall<Riscv128>::invoke<int>(GetegidSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getegid();
}

} /* end namespace RevCPU */ } // end namespace SST