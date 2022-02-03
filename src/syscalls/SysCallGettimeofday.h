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
using GettimeofdaySystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 169>;

template<typename RiscvArchType=Riscv32>
class GettimeofdaySystemCallParameters : public virtual GettimeofdaySystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    timeval * tp;
    void * tzp;

    public:

    using SystemCallParameterInterfaceType = GettimeofdaySystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GettimeofdaySystemCallParameters(timeval tpp, void * tzp)
        : SystemCallParameterInterfaceType(), tp(tpp), tzp(tzp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, timeval * & param) {
        if(parameter_index == 0) {
            param = tp;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, void * & param) {
        if(parameter_index == 0) {
            param = tzp;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using GettimeofdaySystemCallInterfaceType = SystemCallInterface<RiscvArchType, 169>;

template<typename RiscvArchType=Riscv32>
class GettimeofdaySystemCall : public virtual GettimeofdaySystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GettimeofdaySystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    GettimeofdaySystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, ssize_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF