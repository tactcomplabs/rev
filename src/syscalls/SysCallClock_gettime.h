//
// SysCallClock_gettime.h
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
using Clock_gettimeSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 172>;

template<typename RiscvArchType=Riscv32>
class Clock_gettimeSystemCallParameters : public virtual Clock_gettimeSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    clockid_t clkid;
    timespec * tp;

    public:

    using SystemCallParameterInterfaceType = Clock_gettimeSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    Clock_gettimeSystemCallParameters(clockid_t clockid, timespec * tsp)
        : SystemCallParameterInterfaceType(), clkid(clockid), tp(tsp) {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, clockid_t& param) {
        if(parameter_index = 0) {
            param = clkid;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, timespec* & param) {
        if(parameter_index == 1) {
            param = tp;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using Clock_gettimeSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 172>;

template<typename RiscvArchType=Riscv32>
class Clock_gettimeSystemCall : public virtual Clock_gettimeSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = Clock_gettimeSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    Clock_gettimeSystemCall() : SystemCallInterfaceType() {}

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