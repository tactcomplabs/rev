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

#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool GettimeofdayParameters<Riscv32>::get<timeval*>(const size_t parameter_index, timeval * & param) {
    if(parameter_index == 0) {
        param = tp;
        return true;
    }

    return false;
}

template<>
template<>
bool GettimeofdayParameters<Riscv64>::get<timeval*>(const size_t parameter_index, timeval * & param) {
    if(parameter_index == 0) {
        param = tp;
        return true;
    }

    return false;
}

template<>
template<>
bool GettimeofdayParameters<Riscv128>::get<timeval*>(const size_t parameter_index, timeval * & param) {
    if(parameter_index == 0) {
        param = tp;
        return true;
    }

    return false;
}

template<>
template<>
bool GettimeofdayParameters<Riscv32>::get<void*>(const size_t parameter_index, void * & param) {
    if(parameter_index == 0) {
        param = tzp;
        return true;
    }

    return false;
}

template<>
template<>
bool GettimeofdayParameters<Riscv64>::get<void*>(const size_t parameter_index, void * & param) {
    if(parameter_index == 0) {
        param = tzp;
        return true;
    }

    return false;
}

template<>
template<>
bool GettimeofdayParameters<Riscv128>::get<void*>(const size_t parameter_index, void * & param) {
    if(parameter_index == 0) {
        param = tzp;
        return true;
    }

    return false;
}

template<>
template<>
void Gettimeofday<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
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
void Gettimeofday<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
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
void Gettimeofday<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
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
