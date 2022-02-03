//
// SysCallGetrlimit.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETRLIMIT_H__
#define __SYSTEMCALLGETRLIMIT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetrlimitSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 163>;

template<typename RiscvArchType=Riscv32>
class GetrlimitSystemCallParameters : public virtual GetrlimitSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int resource;
    rlimit * rlp;

    public:

    using SystemCallParameterInterfaceType = GetrlimitSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetrlimitSystemCallParameters(int resourcep, rlimit * rlpp)
        : SystemCallParameterInterfaceType(), resource(resourcep), rlp(rlpp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = resource;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, rlimit* & param) {
        if(parameter_index == 1) {
            param = rlp;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using GetrlimitSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 163>;

template<typename RiscvArchType=Riscv32>
class GetrlimitSystemCall : public virtual GetrlimitSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetrlimitSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    GetrlimitSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, std::string & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF