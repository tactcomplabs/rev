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
#ifndef __SYSTEMCALLMUNMAP_H__
#define __SYSTEMCALLMUNMAP_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MunmapInterfaceType = SystemCallInterfaceCode<RiscvArchType, 215>;

template<typename RiscvArchType=Riscv32>
class MunmapParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * addr;
    size_t len;

    public:

    MunmapParameters(void * addrp, size_t lenp)
        : addr(addrp), len(lenp) {}

    size_t count() override { return 6UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Munmap : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MunmapInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Munmap() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
