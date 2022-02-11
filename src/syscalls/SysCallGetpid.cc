//
// SysCallGetgid.cc
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
bool GetpidParameters<Riscv32>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<>
template<>
bool GetpidParameters<Riscv64>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<>
template<>
bool GetpidParameters<Riscv128>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<>
template<>
void Getpid<Riscv32>::invoke<pid_t>(Getpid<Riscv32>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = true;
    value = getpid();
}

template<>
template<>
void Getpid<Riscv64>::invoke<pid_t>(Getpid<Riscv64>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = true;
    value = getpid();
}

template<>
template<>
void Getpid<Riscv128>::invoke<pid_t>(Getpid<Riscv128>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = true;
    value = getpid();
}

} /* end namespace RevCPU */ } // end namespace SST
