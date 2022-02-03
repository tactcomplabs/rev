//
// SysCallGetrusage.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETRUSAGE_H__
#define __SYSTEMCALLGETRUSAGE_H__

#include "SystemCallInterface.h"
#include <type_traits>

#include <sys/resource.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetrusageSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 172>;


template<typename RiscvArchType=Riscv32>
class GetrusageSystemCallParameters : public virtual GetrusageSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int who;
    rusage * r_usage;

    public:

    using SystemCallParameterInterfaceType = GetrusageSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetrusageSystemCallParameters(int who, rusage * rusage)
        : SystemCallParameterInterfaceType(), who(whor), r_usage(rusage) {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index = 0) {
            param = who;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, rusage* & param) {
        if(parameter_index == 1) {
            param = r_usage;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using GetrusageSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 172>;

template<typename RiscvArchType=Riscv32>
class GetrusageSystemCall : public virtual GetrusageSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetrusageSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    GetrusageSystemCall() : SystemCallInterfaceType() {}

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