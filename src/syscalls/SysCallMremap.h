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
#ifndef __SYSTEMCALLMREMAP_H__
#define __SYSTEMCALLMREMAP_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MremapInterfaceType = SystemCallInterfaceCode<RiscvArchType, 216>;

template<typename RiscvArchType=Riscv32>
class MremapParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * oldaddr;
    void * newaddr;
    size_t oldsize;
    size_t newsize;
    int flags;

    public:

    MremapParameters(void * addrp, size_t oldsz, size_t newsz, int flagsp)
        : oldaddr(addrp), oldsize(oldsz), newsize(newsz), flags(flagsp), newaddr(NULL) {}

    MremapParameters(void * oaddr, size_t oldsz, size_t newsz, int flagsp, void * naddr)
        : oldaddr(oaddr), oldsize(oldsz), newsize(newsz), flags(flagsp), newaddr(naddr) {}

    size_t count() override { return (newaddr == NULL) ? 5UL : 6UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Mremap : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MremapInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Mremap() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
