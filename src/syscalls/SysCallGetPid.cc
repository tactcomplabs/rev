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

template<>
template<>
void Getpid<Riscv32>::invoke(Getpid<Riscv32>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    value = getpid();
}

template<>
template<>
void Getpid<Riscv64>::invoke(Getpid<Riscv64>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    value = getpid();
}

template<>
template<>
void Getpid<Riscv128>::invoke(Getpid<Riscv128>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = parameters.count() == 0;
    value = getpid();
}

} /* end namespace RevCPU */ } // end namespace SST