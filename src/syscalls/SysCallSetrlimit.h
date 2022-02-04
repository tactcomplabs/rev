//
// SysCallSetrlimit.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSETRLIMIT_H__
#define __SYSTEMCALLSETRLIMIT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SetrlimitParametersInterfaceType = SystemCallInterface<RiscvArchType, 164>;

template<typename RiscvArchType=Riscv32>
class SetrlimitParameters : public virtual SetrlimitParametersInterfaceType<RiscvArchType> {
    
    private:

    int resource;
    rlimit * rlp;

    public:

    using SystemCallParameterInterfaceType = SetrlimitParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    SetrlimitParameters(int resourcep, rlimit * rlpp)
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
        if(parameter_index == 0) {
            param = rlp;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using SetrlimitInterfaceType = SystemCallInterface<RiscvArchType, 164>;

template<typename RiscvArchType=Riscv32>
class Setrlimit : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SetrlimitInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Setrlimit() : SystemCallInterfaceType() {}

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