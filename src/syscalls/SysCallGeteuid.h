//
// SysCallGeteuid.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETEUID_H__
#define __SYSTEMCALLGETEUID_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GeteuidSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 175>;


template<typename RiscvArchType=Riscv32>
class GeteuidSystemCallParameters : public virtual GeteuidSystemCallParametersInterfaceType<RiscvArchType> {
    
    public:

    using SystemCallParameterInterfaceType = GeteuidSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GeteuidSystemCallParameters(const void_t stat) : SystemCallParameterInterfaceType() {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_t& param) {
        return true;
    }
};

template<typename RiscvArchType=Riscv32>
using GeteuidSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 175>;

template<typename RiscvArchType=Riscv32>
class GeteuidSystemCall : public virtual GeteuidSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GeteuidSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    GeteuidSystemCall() : SystemCallInterfaceType() {}

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