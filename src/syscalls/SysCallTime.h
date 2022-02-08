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
using TimeInterfaceType = SystemCallInterface<RiscvArchType, 1062>;

template<typename RiscvArchType=Riscv32>
class TimeParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    time_t * tloc;

    public:

    using SystemCallParameterInterfaceType = TimeParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    TimeParameters(time_t * tlocp)
        : SystemCallParameterInterfaceType(), tloc(tlocp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, time_t* & param) {
        if(parameter_index == 0) {
            param = tloc;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Time : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TimeInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Time() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, time_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
