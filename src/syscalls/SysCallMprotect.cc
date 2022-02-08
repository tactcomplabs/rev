//
// SysCallMmap.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMprotect.h"

#include <sys/mman.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool MprotectParameters<RiscvArchType>::get(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool MprotectParameters<RiscvArchType>::get(const size_t parameter_index, size_t & param) {
    if(parameter_index == 1) {
        param = len;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool MprotectParameters<RiscvArchType>::get(const size_t parameter_index, int & param) {
    if(parameter_index == 3) {
        param = prot;
        return true;
    }

    return false;
}

template<>
template<>
void Mprotect<Riscv32>::invoke<int>(Mprotect<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 3) {
        void * addr;
        size_t len;
        int prot;

        bool hasargs[3] = { false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);
        hasargs[2] = parameters.get<int>(2, prot);

        if(hasargs[0] && hasargs[1] && hasargs[2]) {
            success = true;
            value = mprotect(addr, len, prot);
        }
    }
}

template<>
template<>
void Mprotect<Riscv64>::invoke<int>(Mprotect<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 3) {
        void * addr;
        size_t len;
        int prot;

        bool hasargs[3] = { false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);
        hasargs[2] = parameters.get<int>(2, prot);

        if(hasargs[0] && hasargs[1] && hasargs[2]) {
            success = true;
            value = mprotect(addr, len, prot);
        }
    }
}

template<>
template<>
void Mprotect<Riscv128>::invoke<int>(Mprotect<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

    if(parameters.count() == 3) {
        void * addr;
        size_t len;
        int prot;

        bool hasargs[3] = { false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);
        hasargs[2] = parameters.get<int>(2, prot);

        if(hasargs[0] && hasargs[1] && hasargs[2]) {
            success = true;
            value = mprotect(addr, len, prot);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
