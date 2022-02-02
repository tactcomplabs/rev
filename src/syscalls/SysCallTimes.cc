//
// SysCallTimes.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallTimes.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void TimesSystemCall<Riscv32>::invoke<clock_t>(TimesSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
    if(parameters.count() == 1) {
        tms * tp;

        bool has_values = parameters.get<tms *>(0, tp);

        if(has_values) {
            success = true;
            value = times(tp);
        }
    }
}

template<>
template<>
void TimesSystemCall<Riscv64>::invoke<clock_t>(TimesSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
    if(parameters.count() == 1) {
        tms * tp;

        bool has_values = parameters.get<tms *>(0, tp);

        if(has_values) {
            success = true;
            value = times(tp);
        }
    }
}

template<>
template<>
void TimesSystemCall<Riscv128>::invoke<clock_t>(TimesSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
    if(parameters.count() == 1) {
        tms * tp;

        bool has_values = parameters.get<tms *>(0, tp);

        if(has_values) {
            success = true;
            value = times(tp);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST