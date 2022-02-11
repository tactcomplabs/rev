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
bool GetgidParameters<Riscv32>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<>
template<>
bool GetgidParameters<Riscv64>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<>
template<>
bool GetgidParameters<Riscv128>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<>
template<>
void Getgid<Riscv32>::invoke<gid_t>(Getgid<Riscv32>::SystemCallParameterInterfaceType & parameters, gid_t & value) {
    success = true;
    value = getgid();
}

template<>
template<>
void Getgid<Riscv64>::invoke<gid_t>(Getgid<Riscv64>::SystemCallParameterInterfaceType & parameters, gid_t & value) {
    success = true;
    value = getgid();
}

template<>
template<>
void Getgid<Riscv128>::invoke<gid_t>(Getgid<Riscv128>::SystemCallParameterInterfaceType & parameters, gid_t & value) {
    success = true;
    value = getgid();
}

} /* end namespace RevCPU */ } // end namespace SST
