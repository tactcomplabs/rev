//
// SysCallMmap.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMMAP_H__
#define __SYSTEMCALLMMAP_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/mman.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MmapInterfaceType = SystemCallInterfaceCode<RiscvArchType, 222>;

template<typename RiscvArchType=Riscv32>
class MmapParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * addr;
    size_t len;
    int prot;
    int flags;
    int fd;
    off_t offset;

    public:

    MmapParameters(void * addrp, size_t lenp, int protp, int flagsp, int fdp, off_t offsetp)
        : addr(addrp), len(lenp), prot(protp), flags(flagsp), fd(fdp), offset(offsetp) {}

    size_t count() override { return 6UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Mmap : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MmapInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Mmap() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
