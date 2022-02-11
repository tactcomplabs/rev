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
using GetrusageInterfaceType = SystemCallInterfaceCode<RiscvArchType, 165>;

template<typename RiscvArchType=Riscv32>
class GetrusageParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int who;
    rusage * r_usage;

    public:

    GetrusageParameters(int rwho, rusage * rusage)
        : who(rwho), r_usage(rusage) {}

    size_t count() override { return 0UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Getrusage : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetrusageInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Getrusage() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
