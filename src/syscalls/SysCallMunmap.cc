//
// SysCallMmap.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMunmap.h"
#include <sys/mman.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool MunmapParameters<RiscvArchType>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool MunmapParameters<RiscvArchType>::get<size_t>(const size_t parameter_index, size_t & param) {
    if(parameter_index == 1) {
        param = len;
        return true;
    }

    return false;
}

template<>
template<>
void Munmap<Riscv32>::invoke<int>(Munmap<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 2) {
        void * addr;
        size_t len;

        bool hasargs[2] = { false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);

        if(hasargs[0] && hasargs[1]) {
            success = true;
            value = munmap(addr, len);
        }
    }
}

template<>
template<>
void Munmap<Riscv64>::invoke<int>(Munmap<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 2) {
        void * addr;
        size_t len;

        bool hasargs[2] = { false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);

        if(hasargs[0] && hasargs[1]) {
            success = true;
            value = munmap(addr, len);
        }
    }
}

template<>
template<>
void Munmap<Riscv128>::invoke<int>(Munmap<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 2) {
        void * addr;
        size_t len;

        bool hasargs[2] = { false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);

        if(hasargs[0] && hasargs[1]) {
            success = true;
            value = munmap(addr, len);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
