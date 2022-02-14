//
// SysCallGettimeofday.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETTIMEOFDAY_H__
#define __SYSTEMCALLGETTIMEOFDAY_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <sys/time.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GettimeofdayInterfaceType = SystemCallInterfaceCode<RiscvArchType, 169>;

template<typename RiscvArchType=Riscv32>
class GettimeofdayParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    timeval * tp;
    void * tzp;

    public:

    GettimeofdayParameters(timeval tpp, void * tzp)
        : tp(tpp), tzp(tzp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Gettimeofday : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GettimeofdayInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Gettimeofday() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
