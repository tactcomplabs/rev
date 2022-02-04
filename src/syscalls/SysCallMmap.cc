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
#include <sys/mman.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Mmap<Riscv32>::invoke<void_t>(Mmap<Riscv32>::SystemCallParameterInterfaceType & parameters, void* & value) {

    if(parameters.count() == 6) {
        void * addr;
        size_t len;
        int prot;
        int flags;
        int fd;
        off_t offset;

        bool hasargs[6] = { false, false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);
        hasargs[2] = parameters.get<int>(2, prot);
        hasargs[3] = parameters.get<int>(3, flags);
        hasargs[4] = parameters.get<int>(4, fd);
        hasargs[5] = parameters.get<int>(5, offset);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4] && hasargs[5]) {
            success = true;
            value = mmap(addr, len, prot, flags, fd, offset);
        }
    }
}

template<>
template<>
void Mmap<Riscv64>::invoke<void_t>(Mmap<Riscv64>::SystemCallParameterInterfaceType & parameters, void* & value) {

    if(parameters.count() == 6) {
        void * addr;
        size_t len;
        int prot;
        int flags;
        int fd;
        off_t offset;

        bool hasargs[6] = { false, false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);
        hasargs[2] = parameters.get<int>(2, prot);
        hasargs[3] = parameters.get<int>(3, flags);
        hasargs[4] = parameters.get<int>(4, fd);
        hasargs[5] = parameters.get<int>(5, offset);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4] && hasargs[5]) {
            success = true;
            value = mmap(addr, len, prot, flags, fd, offset);
        }
    }
}

template<>
template<>
void Mmap<Riscv128>::invoke<void_t>(Mmap<Riscv128>::SystemCallParameterInterfaceType & parameters, void* & value) {

    if(parameters.count() == 6) {
        void * addr;
        size_t len;
        int prot;
        int flags;
        int fd;
        off_t offset;

        bool hasargs[6] = { false, false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, addr);
        hasargs[1] = parameters.get<size_t>(1, len);
        hasargs[2] = parameters.get<int>(2, prot);
        hasargs[3] = parameters.get<int>(3, flags);
        hasargs[4] = parameters.get<int>(4, fd);
        hasargs[5] = parameters.get<int>(5, offset);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4] && hasargs[5]) {
            success = true;
            value = mmap(addr, len, prot, flags, fd, offset);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST