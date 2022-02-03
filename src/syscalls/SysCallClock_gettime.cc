//
// SysCallClock_gettime.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallClock_gettime.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Clock_gettimeSystemCall<Riscv32>::invoke<int>(Clock_gettimeSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parmaters.size() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parmeters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

template<>
template<>
void Clock_gettimeSystemCall<Riscv64>::invoke<int>(Clock_gettimeSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parmaters.size() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parmeters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

template<>
template<>
void Clock_gettimeSystemCall<Riscv128>::invoke<int>(Clock_gettimeSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parmaters.size() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parmeters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

} /* end namespace RevCPU */ } // end namespace SST