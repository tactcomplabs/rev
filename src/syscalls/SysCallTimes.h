//
// SysCallTimes.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLTIMES_H__
#define __SYSTEMCALLTIMES_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <sys/times.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using TimesSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 153>;

template<typename RiscvArchType=Riscv32>
class TimesSystemCallParameters : public virtual TimesSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    tms * tp;

    public:

    using SystemCallParameterInterfaceType = TimesSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    TimesSystemCallParameters(tms * tpp)
        : SystemCallParameterInterfaceType(), tp(tpp) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, tms * & param) {
        if(parameter_index == 0) {
            param = tp;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using TimesSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 153>;

template<typename RiscvArchType=Riscv32>
class TimesSystemCall : public virtual TimesSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TimesSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    TimesSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, clock_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF