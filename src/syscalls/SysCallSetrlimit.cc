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
bool SetrlimitParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
   if(parameter_index == 0) {
       param = resource;
       return true;
   }

   return false;
}
template<>
template<>
bool SetrlimitParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
   if(parameter_index == 0) {
       param = resource;
       return true;
   }

   return false;
}

template<>
template<>
bool SetrlimitParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
   if(parameter_index == 0) {
       param = resource;
       return true;
   }

   return false;
}

template<>
template<>
bool SetrlimitParameters<Riscv32>::get<rlimit*>(const size_t parameter_index, rlimit* & param) {
    if(parameter_index == 0) {
       param = rlp;
       return true;
    }

    return false;
}

template<>
template<>
bool SetrlimitParameters<Riscv64>::get<rlimit*>(const size_t parameter_index, rlimit* & param) {
    if(parameter_index == 0) {
       param = rlp;
       return true;
    }

    return false;
}

template<>
template<>
bool SetrlimitParameters<Riscv128>::get<rlimit*>(const size_t parameter_index, rlimit* & param) {
    if(parameter_index == 0) {
       param = rlp;
       return true;
    }

    return false;
}

template<>
template<>
void Setrlimit<Riscv32>::invoke<int>(Setrlimit<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int resource;
        rlimit * rlp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, resource);
        has_values[1] = parameters.get<rlimit*>(1, rlp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = setrlimit(resource, rlp);
        }
    }
}

template<>
template<>
void Setrlimit<Riscv64>::invoke<int>(Setrlimit<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
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
void Setrlimit<Riscv128>::invoke<int>(Setrlimit<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
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
