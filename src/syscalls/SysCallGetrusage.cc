//
// SysCallGetrusage.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetrusage.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool GetrusageParameters<RiscvArchType>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index = 0) {
        param = who;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool GetrusageParameters<RiscvArchType>::get<rusage*>(const size_t parameter_index, rusage* & param) {
    if(parameter_index == 1) {
        param = r_usage;
        return true;
    }

    return false;
}


template<>
template<>
void Getrusage<Riscv32>::invoke<int>(Getrusage<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        int who;
        rusage * r_usage;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, who);
        has_values[1] = parameters.get<rusage*>(1, r_usage);
        success = true;
        value = getrusage(who, r_usage);
    }
}

template<>
template<>
void Getrusage<Riscv64>::invoke<int>(Getrusage<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        int who;
        rusage * r_usage;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, who);
        has_values[1] = parameters.get<rusage*>(1, r_usage);
        success = true;
        value = getrusage(who, r_usage);
    }
}

template<>
template<>
void Getrusage<Riscv128>::invoke<int>(Getrusage<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
     if(parameters.count() == 2) {
        int who;
        rusage * r_usage;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, who);
        has_values[1] = parameters.get<rusage*>(1, r_usage);
        success = true;
        value = getrusage(who, r_usage);
    }
}

} /* end namespace RevCPU */ } // end namespace SST
