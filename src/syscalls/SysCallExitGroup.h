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
using ExitgroupInterfaceType = SystemCallInterfaceCode<RiscvArchType, 94>;

template<typename RiscvArchType=Riscv32>
class ExitgroupParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int status;

    public:

    ExitgroupParameters(const int stat) : status(stat) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Exitgroup : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = ExitgroupInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Exitgroup() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
