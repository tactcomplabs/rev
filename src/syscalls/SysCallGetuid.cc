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

namespace SST { namespace RevCPU {

template<>
template<>
void Getuid<Riscv32>::invoke<int>(Getuid<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getuid();
}

template<>
template<>
void Getuid<Riscv64>::invoke<int>(Getuid<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getuid();
}

template<>
template<>
void Getuid<Riscv128>::invoke<int>(Getuid<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    success = true;
    value = getuid();
}

} /* end namespace RevCPU */ } // end namespace SST