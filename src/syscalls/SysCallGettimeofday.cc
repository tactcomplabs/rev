//
// SysCallGettimeofday.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGettimeofday.h"
#include <algorithm>

#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
void GettimeofdaySystemCall<Riscv32>::invoke<int>(GettimeofdaySystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
   if(parameters.count() == 2) {

        timeval * tp;
        void * tzp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<timeval *>(0, tp);
        has_values[1] = parameters.get<void *>(0, tzp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = gettimeofday(tp, tzp);
        }
    }
}

template<>
template<>
void GettimeofdaySystemCall<Riscv64>::invoke<int>(GettimeofdaySystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
   if(parameters.count() == 2) {

        timeval * tp;
        void * tzp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<timeval *>(0, tp);
        has_values[1] = parameters.get<void *>(0, tzp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = gettimeofday(tp, tzp);
        }
    }
}

template<>
template<>
void GettimeofdaySystemCall<Riscv128>::invoke<int>(GettimeofdaySystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
   if(parameters.count() == 2) {

        timeval * tp;
        void * tzp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<timeval *>(0, tp);
        has_values[1] = parameters.get<void *>(0, tzp);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = gettimeofday(tp, tzp);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST