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

template<>
template<>
void GetrusageSystemCall<Riscv32>::invoke<int>(GetrusageSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parmaters.size() == 2) {
        int who;
        rusage * r_usage;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, who);
        has_values[1] = parmeters.get<rusage*>(1, r_usage);
        success = true;
        value = rusage(who, r_usage);
    }
}

template<>
template<>
void GetrusageSystemCall<Riscv64>::invoke<int>(GetrusageSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parmaters.size() == 2) {
        int who;
        rusage * r_usage;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, who);
        has_values[1] = parmeters.get<rusage*>(1, r_usage);
        success = true;
        value = rusage(who, r_usage);
    }
}

template<>
template<>
void GetrusageSystemCall<Riscv128>::invoke<int>(GetrusageSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parmaters.size() == 2) {
        int who;
        rusage * r_usage;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, who);
        has_values[1] = parmeters.get<rusage*>(1, r_usage);
        success = true;
        value = rusage(who, r_usage);
    }
}

} /* end namespace RevCPU */ } // end namespace SST