//
// SysCallDup3.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallDup3.h"

#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool Dup3Parameters<RiscvArchType>::get(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 1) {
        param = fildes;
        return true;
    }
    else if(parameter_index == 2) {
        param = fildes;
        return true;
    }

    return false;
}


template<>
template<>
void Dup3<Riscv32>::invoke<int>(Dup3<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int ofd, nfd, flags;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, ofd);
        has_values[1] = parameters.get<int>(1, nfd);
        has_values[2] = parameters.get<int>(2, flags);

        if(has_values[0] && has_values[1]  && has_values[2]) {
            success = true;
            value = dup3(ofd, nfd, flags);
        }
    }
}

template<>
template<>
void Dup3<Riscv64>::invoke<int>(Dup3<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int ofd, nfd, flags;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, ofd);
        has_values[1] = parameters.get<int>(1, nfd);
        has_values[2] = parameters.get<int>(2, flags);

        if(has_values[0] && has_values[1]  && has_values[2]) {
            success = true;
            value = dup3(ofd, nfd, flags);
        }
    }
}

template<>
template<>
void Dup3<Riscv128>::invoke<int>(Dup3<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int ofd, nfd, flags;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, ofd);
        has_values[1] = parameters.get<int>(1, nfd);
        has_values[2] = parameters.get<int>(2, flags);

        if(has_values[0] && has_values[1]  && has_values[2]) {
            success = true;
            value = dup3(ofd, nfd, flags);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
