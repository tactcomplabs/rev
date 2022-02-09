//
// SysCallGetegid.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetegid.h"

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool GetegidParameters<RiscvArchType>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}


template<>
template<>
void Getegid<Riscv32>::invoke<gid_t>(Getegid<Riscv32>::SystemCallParameterInterfaceType & parameters, gid_t & value) {
    success = true;
    value = getegid();
}

template<>
template<>
void Getegid<Riscv64>::invoke<gid_t>(Getegid<Riscv64>::SystemCallParameterInterfaceType & parameters, gid_t & value) {
    success = true;
    value = getegid();
}

template<>
template<>
void Getegid<Riscv128>::invoke<gid_t>(Getegid<Riscv128>::SystemCallParameterInterfaceType & parameters, gid_t & value) {
    success = true;
    value = getegid();
}

} /* end namespace RevCPU */ } // end namespace SST
