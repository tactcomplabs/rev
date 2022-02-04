//
// SysCallGetgid.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETGID_H__
#define __SYSTEMCALLGETGID_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetgidInterfaceType = SystemCallInterface<RiscvArchType, 176>;


template<typename RiscvArchType=Riscv32>
class GetgidParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    public:

    using SystemCallParameterInterfaceType = GetgidParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetgidParameters(const void_t stat) : SystemCallParameterInterfaceType() {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_t& param) {
        return true;
    }
};

template<typename RiscvArchType=Riscv32>
class Getgid : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetgidInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Getgid() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
    
    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, gid_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF