//
// SysCallStatx.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSTATX_H__
#define __SYSTEMCALLSTATX_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

using statx_t = struct statx;

template<typename RiscvArchType=Riscv32>
using StatxInterfaceType = SystemCallInterfaceCode<RiscvArchType, 291>;

template<typename RiscvArchType=Riscv32>
class StatxParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int dirfd;
    const char * pathname;
    int flags;
    unsigned int mask;
    statx_t * statxbuf;

    public:

    StatxParameters(int dirfdp, const char * pathnamep, int flagsp, unsigned int maskp, statx_t * statxbufp)
        : dirfd(dirfdp), flags(flagsp), mask(maskp), statxbuf(statxbuf) {}

    size_t count() override { return 5UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Statx : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = StatxInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Statx() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
