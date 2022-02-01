//
// SysCallGetpid.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETPID_H__
#define __SYSTEMCALLGETPID_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetpidSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 172>;


template<typename RiscvArchType=Riscv32>
class GetpidSystemCallParameters : public virtual GetpidSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int status;

    public:

    using SystemCallParameterInterfaceType = GetpidSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetpidSystemCallParameters(const int stat) : SystemCallParameterInterfaceType(), status(stat) {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_t& param) {
        return true;
    }
};

template<typename RiscvArchType=Riscv32>
using GetpidSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 172>;

template<typename RiscvArchType=Riscv32>
class GetpidSystemCall : public virtual GetpidSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetpidSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    GetpidSystemCall() : SystemCallInterfaceType() {}

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