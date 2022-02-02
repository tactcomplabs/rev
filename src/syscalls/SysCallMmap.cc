//
// SysCallMmap.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMmap.h"
#include <stdlib.h>

namespace SST { namespace RevCPU {

template<>
template<>
void MmapSystemCall<Riscv32>::invoke<void_t>(MmapSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value) {

    if(parameters.count() == 6) {
        void * addr;
        size_t len;
        int prot;
        int flags;
        int fd;
        off_t offset;

        bool hasargs[6] = { false, false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(addr);
        hasargs[1] = parameters.get<size_t>(len);
        hasargs[2] = parameters.get<int>(prot);
        hasargs[3] = parameters.get<int>(flags);
        hasargs[4] = parameters.get<int>(fd);
        hasargs[5] = parameters.get<int>(offset);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4] && hasargs[5]) {
            success = true;
            value = mmap(addr, len, prot, flags, fd, offset);
        }
    }
}

template<>
template<>
void MmapSystemCall<Riscv64>::invoke<void_t>(MmapSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value) {

    if(parameters.count() == 6) {
        void * addr;
        size_t len;
        int prot;
        int flags;
        int fd;
        off_t offset;

        bool hasargs[6] = { false, false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(addr);
        hasargs[1] = parameters.get<size_t>(len);
        hasargs[2] = parameters.get<int>(prot);
        hasargs[3] = parameters.get<int>(flags);
        hasargs[4] = parameters.get<int>(fd);
        hasargs[5] = parameters.get<int>(offset);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4] && hasargs[5]) {
            success = true;
            value = mmap(addr, len, prot, flags, fd, offset);
        }
    }
}

template<>
template<>
void MmapSystemCall<Riscv128>::invoke<void_t>(MmapSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value) {

    if(parameters.count() == 6) {
        void * addr;
        size_t len;
        int prot;
        int flags;
        int fd;
        off_t offset;

        bool hasargs[6] = { false, false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(addr);
        hasargs[1] = parameters.get<size_t>(len);
        hasargs[2] = parameters.get<int>(prot);
        hasargs[3] = parameters.get<int>(flags);
        hasargs[4] = parameters.get<int>(fd);
        hasargs[5] = parameters.get<int>(offset);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4] && hasargs[5]) {
            success = true;
            value = mmap(addr, len, prot, flags, fd, offset);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST