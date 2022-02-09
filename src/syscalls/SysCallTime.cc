//
// SysCallTime.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallTime.h"

#include <unistd.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool TimeParameters<RiscvArchType>::get<time_t*>(const size_t parameter_index, time_t* & param) {
    if(parameter_index == 0) {
        param = tloc;
        return true;
    }

    return false;
}

template<>
template<>
void Time<Riscv32>::invoke<time_t>(Time<Riscv32>::SystemCallParameterInterfaceType & parameters, time_t & value) {
    if(parameters.count() == 1) {

        time_t * tloc;
        
        bool has_values = parameters.get<time_t*>(0, tloc);

        if(has_values) {
            success = true;
            value = time(tloc);
        }
    }
}

template<>
template<>
void Time<Riscv64>::invoke<time_t>(Time<Riscv64>::SystemCallParameterInterfaceType & parameters, time_t & value) {
    if(parameters.count() == 1) {

        time_t * tloc;
        
        bool has_values = parameters.get<time_t*>(0, tloc);

        if(has_values) {
            success = true;
            value = time(tloc);
        }
    }
}

template<>
template<>
void Time<Riscv128>::invoke<time_t>(Time<Riscv128>::SystemCallParameterInterfaceType & parameters, time_t & value) {
    if(parameters.count() == 1) {

        time_t * tloc;
        
        bool has_values = parameters.get<time_t*>(0, tloc);

        if(has_values) {
            success = true;
            value = time(tloc);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
