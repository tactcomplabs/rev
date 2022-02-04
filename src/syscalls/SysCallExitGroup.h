//
// SysCallExitGroup.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLEXITGROUP_H__
#define __SYSTEMCALLEXITGROUP_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using ExitgroupInterfaceType = SystemCallInterface<RiscvArchType, 94>;

template<typename RiscvArchType=Riscv32>
class ExitgroupParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int status;

    public:

    using SystemCallParameterInterfaceType = ExitgroupParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    ExitgroupParameters(const int stat) : SystemCallParameterInterfaceType(), status(stat) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = status;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Exitgroup : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = ExitgroupInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Exitgroup() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, void_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF