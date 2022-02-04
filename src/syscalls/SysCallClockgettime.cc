//
// SysCallClockgettime.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallClockgettime.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Clockgettime<Riscv32>::invoke<int>(Clockgettime<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parameters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

template<>
template<>
void Clockgettime<Riscv64>::invoke<int>(Clockgettime<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parameters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

template<>
template<>
void Clockgettime<Riscv128>::invoke<int>(Clockgettime<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parameters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

} /* end namespace RevCPU */ } // end namespace SST