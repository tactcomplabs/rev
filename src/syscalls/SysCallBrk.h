//
// SysCallBrk.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLBRK_H__
#define __SYSTEMCALLBRK_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using BrkInterfaceType = SystemCallInterfaceCode<RiscvArchType, 214>;

template<typename RiscvArchType=Riscv32>
class BrkParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void* addr;

    public:

    BrkParameters(void * addr_i)
        : addr(addr_i) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Brk : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = BrkInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Brk() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
