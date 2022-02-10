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

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool MmapParameters<RiscvArchType>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 0) {
        param = addr;
        return true;
    }

    return false;
}
        
template<typename RiscvArchType>
template<>
bool MmapParameters<RiscvArchType>::get<size_t>(const size_t parameter_index, size_t & param) {
    if(parameter_index == 1) {
        param = len;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool MmapParameters<RiscvArchType>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 2) {
        param = prot;
        return true;
    }
    else if(parameter_index == 3) {
        param = flags;
        return true;
    }
    else if(parameter_index == 4) {
        param = fd;
        return true;
    }
    else if(parameter_index == 5) {
        param = offset;
        return true;
    }

    return false;
}

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
