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
using TimesInterfaceType = SystemCallInterfaceCode<RiscvArchType, 153>;

template<typename RiscvArchType=Riscv32>
class TimesParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    tms * tp;

    public:

    TimesParameters(tms * tpp)
        : tp(tpp) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Times : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TimesInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Times() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
