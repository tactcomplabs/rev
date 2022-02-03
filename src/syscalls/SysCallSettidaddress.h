//
// SysCallSettidaddress.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSETTIDADDRESS_H__
#define __SYSTEMCALLSETTIDADDRESS_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SettidaddressSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class SettidaddressSystemCallParameters : public virtual SettidaddressSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int* tidptr;

    public:

    using SystemCallParameterInterfaceType = SettidaddressSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    SettidaddressSystemCallParameters(int * tidptrp)
        : SystemCallParameterInterfaceType(), tidptr(tidptrp) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int* & param) {
        if(parameter_index == 0) {
            param = tidptr;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using SettidaddressSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class SettidaddressSystemCall : public virtual SettidaddressSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SettidaddressSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    SettidaddressSystemCall() : SystemCallInterfaceType() {}

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