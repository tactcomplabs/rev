//
// SysCallClockgettime.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLCLOCKGETTIME_H__
#define __SYSTEMCALLCLOCKGETTIME_H__

#include "SystemCallInterface.h"
#include <type_traits>

#include <sys/resource.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using ClockgettimeInterfaceType = SystemCallInterfaceCode<RiscvArchType, 113>;

template<typename RiscvArchType=Riscv32>
class ClockgettimeParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:
    
    clockid_t clkid;
    timespec * tp;

    public:

    ClockgettimeParameters(clockid_t clockid, timespec * tsp)
        : clkid(clockid), tp(tsp) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Clockgettime : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = ClockgettimeInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Clockgettime() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
