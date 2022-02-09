//
// SysCallMadvise.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMadvise.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {


    template<typename RiscvArchType>
    template<>
    bool MadviseParameters<RiscvArchType>::get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = length;
            return true;
        }

        return false;
    }

    template<typename RiscvArchType>
    template<>
    bool MadviseParameters<RiscvArchType>::get(const size_t parameter_index, int & param) {
        if(parameter_index == 2) {
            param = advice;
            return true;
        }

        return false;
    }

    template<typename RiscvArchType>
    template<>
    bool MadviseParameters<RiscvArchType>::get(const size_t parameter_index, void * & param) {
        if(parameter_index == 0) {
            param = addr;
            return true;
        }

        return false;
    }


template<>
template<>
void Madvise<Riscv32>::invoke<int>(Madvise<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        void * addr;
        size_t length;
        int advice;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<void*>(0, addr);
        has_values[1] = parameters.get<size_t>(1, length);
        has_values[2] = parameters.get<int>(2, advice);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = madvise(addr, length, advice);
        }
    }
}

template<>
template<>
void Madvise<Riscv64>::invoke<int>(Madvise<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        void * addr;
        size_t length;
        int advice;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<void*>(0, addr);
        has_values[1] = parameters.get<size_t>(1, length);
        has_values[2] = parameters.get<int>(2, advice);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = madvise(addr, length, advice);
        }
    }
}

template<>
template<>
void Madvise<Riscv128>::invoke<int>(Madvise<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        void * addr;
        size_t length;
        int advice;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<void*>(0, addr);
        has_values[1] = parameters.get<size_t>(1, length);
        has_values[2] = parameters.get<int>(2, advice);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = madvise(addr, length, advice);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
