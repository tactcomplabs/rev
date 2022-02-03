
//
// SysCallSetrlimit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallSetrlimit.h"

#include <unistd.h>
#include <sys/resource.h>

namespace SST { namespace RevCPU {

template<>
template<>
void SetrlimitSystemCall<Riscv32>::invoke<int>(SetrlimitSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit *>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = setrlimit(resource, rlp);
        }
    }
}

template<>
template<>
void SetrlimitSystemCall<Riscv64>::invoke<int>(SetrlimitSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit *>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = setrlimit(resource, rlp);
        }
    }
}

template<>
template<>
void SetrlimitSystemCall<Riscv128>::invoke<int>(SetrlimitSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit *>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = setrlimit(resource, rlp);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST