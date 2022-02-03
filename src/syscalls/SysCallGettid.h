//
// SysCallGettid.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETTID_H__
#define __SYSTEMCALLGETTID_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GettidSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 178>;


template<typename RiscvArchType=Riscv32>
class GettidSystemCallParameters : public virtual GettidSystemCallParametersInterfaceType<RiscvArchType> {
    
    public:

    using SystemCallParameterInterfaceType = GettidSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GettidSystemCallParameters(const void_t stat) : SystemCallParameterInterfaceType() {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_t& param) {
        return true;
    }
};

template<typename RiscvArchType=Riscv32>
using GettidSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 178>;

template<typename RiscvArchType=Riscv32>
class GettidSystemCall : public virtual GettidSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GettidSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    GettidSystemCall() : SystemCallInterfaceType() {}

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