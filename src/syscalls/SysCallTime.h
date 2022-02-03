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
using TimeSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class TimeSystemCallParameters : public virtual TimeSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    time_t * tloc;

    public:

    using SystemCallParameterInterfaceType = TimeSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    TimeSystemCallParameters(time_t * tlocp)
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
using TimeSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class TimeSystemCall : public virtual TimeSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TimeSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    TimeSystemCall() : SystemCallInterfaceType() {}

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