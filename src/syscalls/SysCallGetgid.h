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
using GetgidSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 176>;


template<typename RiscvArchType=Riscv32>
class GetgidSystemCallParameters : public virtual GetgidSystemCallParametersInterfaceType<RiscvArchType> {
    
    public:

    using SystemCallParameterInterfaceType = GetgidSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetgidSystemCallParameters(const void_t stat) : SystemCallParameterInterfaceType() {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_t& param) {
        return true;
    }
};

template<typename RiscvArchType=Riscv32>
using GetgidSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 176>;

template<typename RiscvArchType=Riscv32>
class GetgidSystemCall : public virtual GetgidSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetgidSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    GetgidSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
    
    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, pid_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF