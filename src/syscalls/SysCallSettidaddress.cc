//
// SysCallSettidaddress.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallSettidaddress.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {


template<typename RiscvArchType>
template<>
bool SettidaddressParameters<RiscvArchType>::get<int*>(const size_t parameter_index, int* & param) {
    if(parameter_index == 0) {
        param = tidptr;
        return true;
    }

    return false;
}

template<>
template<>
void Settidaddress<Riscv32>::invoke<int>(Settidaddress<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {

        int * fdptr;
        bool has_values = false;
        has_values = parameters.get<int*>(0, fdptr);

        if(has_values) {
            success = true;
            value = syscall(SYS_set_tid_address, fdptr);
        }
    }
}

template<>
template<>
void Settidaddress<Riscv64>::invoke<int>(Settidaddress<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {

        int * fdptr;
        bool has_values = false;
        has_values = parameters.get<int*>(0, fdptr);

        if(has_values) {
            success = true;
            value = syscall(SYS_set_tid_address, fdptr);
        }
    }
}

template<>
template<>
void Settidaddress<Riscv128>::invoke<int>(Settidaddress<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {

        int * fdptr;
        bool has_values = false;
        has_values = parameters.get<int*>(0, fdptr);

        if(has_values) {
            success = true;
            value = syscall(SYS_set_tid_address, fdptr);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
