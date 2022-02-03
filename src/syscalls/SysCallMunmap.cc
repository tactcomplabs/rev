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
#include <stdlib.h>

namespace SST { namespace RevCPU {

template<>
template<>
void MmapSystemCall<Riscv32>::invoke<int>(MmapSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

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
void MmapSystemCall<Riscv64>::invoke<int>(MmapSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

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
void MunmapSystemCall<Riscv128>::invoke<int>(MmapSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

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