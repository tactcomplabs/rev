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

template<>
template<>
void TimeSystemCall<Riscv32>::invoke<time_t>(TimeSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, time_t & value) {
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
void TimeSystemCall<Riscv64>::invoke<time_t>(TimeSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, time_t & value) {
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
void TimeSystemCall<Riscv128>::invoke<time_t>(TimeSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, time_t & value) {
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