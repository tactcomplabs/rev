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


    template<typename RiscvArchType>
    template<>
    bool Times<RiscvArchType>::get(const size_t parameter_index, tms * & param) {
        if(parameter_index == 0) {
            param = tp;
            return true;
        }

        return false;
    }

template<>
template<>
void Times<Riscv32>::invoke<clock_t>(Times<Riscv32>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
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
void Times<Riscv64>::invoke<clock_t>(Times<Riscv64>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
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
void Times<Riscv128>::invoke<clock_t>(Times<Riscv128>::SystemCallParameterInterfaceType & parameters, clock_t & value) {
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
