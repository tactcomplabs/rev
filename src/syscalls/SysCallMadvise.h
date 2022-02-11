//
// SysCallMadvise.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMADVISE_H__
#define __SYSTEMCALLMADVISE_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <sys/mman.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MadviseInterfaceType = SystemCallInterfaceCode<RiscvArchType, 233>;

template<typename RiscvArchType=Riscv32>
class MadviseParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * addr;
    size_t length;
    int advice;

    public:

    MadviseParameters(void * addr_, size_t length_, int advice_)
        : addr(addr_), length(length_), advice(advice_) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Madvise : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MadviseInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Madvise() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
