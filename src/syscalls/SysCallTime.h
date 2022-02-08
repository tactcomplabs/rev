//
// SysCallTime.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLTIME_H__
#define __SYSTEMCALLTIME_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <time.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using TimeInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1062>;

template<typename RiscvArchType=Riscv32>
class TimeParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    time_t * tloc;

    public:

    TimeParameters(time_t * tlocp)
        : tloc(tlocp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Time : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TimeInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Time() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
