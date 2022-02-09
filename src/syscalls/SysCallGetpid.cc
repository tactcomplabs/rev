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

template<typename RiscvArchType>
template<>
bool Getpid<RiscvArchType>::get<void_t>(const size_t parameter_index, void_t& param) {
    return true;
}

template<typename RiscvArchType>
template<>
void Getpid<RiscvArchType>::invoke<pid_t>(Gitpid<RiscvArchType>::SystemCallParameterInterfaceType & parameters, pid_t & value) {
    success = true;
    value = getpid();
}

} /* end namespace RevCPU */ } // end namespace SST
