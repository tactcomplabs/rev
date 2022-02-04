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
using GetrusageInterfaceType = SystemCallInterface<RiscvArchType, 165>;


template<typename RiscvArchType=Riscv32>
class GetrusageParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int who;
    rusage * r_usage;

    public:

    using SystemCallParameterInterfaceType = GetrusageParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetrusageParameters(int who, rusage * rusage)
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
class Getrusage : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetrusageInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Getrusage() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
    
    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF