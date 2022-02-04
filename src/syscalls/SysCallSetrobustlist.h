//
// SysCallSetrobustlist.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSETROBUSTLIST_H__
#define __SYSTEMCALLSETROBUSTLIST_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SetrobustlistParametersInterfaceType = SystemCallInterface<RiscvArchType, 99>;

template<typename RiscvArchType=Riscv32>
class SetrobustlistParameters : public virtual SetrobustlistParametersInterfaceType<RiscvArchType> {
    
    private:

    robust_list_head * hptr;
    size_t len;

    public:

    using SystemCallParameterInterfaceType = SetrobustlistParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    SetrobustlistParameters(robust_list_head * ptr, size_t lenp)
        : SystemCallParameterInterfaceType(), hptr(ptr), len(lenp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, robust_list_head* & param) {
        if(parameter_index == 0) {
            param = hptr;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = len;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using SetrobustlistInterfaceType = SystemCallInterface<RiscvArchType, 99>;

template<typename RiscvArchType=Riscv32>
class Setrobustlist : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SetrobustlistInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Setrobustlist() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF