//
// SysCallMprotect.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMPROTECT_H__
#define __SYSTEMCALLMPROTECT_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MprotectInterfaceType = SystemCallInterfaceCode<RiscvArchType, 226>;

template<typename RiscvArchType=Riscv32>
class MprotectParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * addr;
    size_t len;
    int prot;

    public:

    MprotectParameters(void * addrp, size_t lenp, int protp)
        : addr(addrp), len(lenp), prot(protp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Mprotect : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MprotectInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Mprotect() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
