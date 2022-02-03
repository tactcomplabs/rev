//
// SysCallAccess.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLACCESS_H__
#define __SYSTEMCALLACCESS_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using AccessSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class AccessSystemCallParameters : public virtual AccessSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    std::string pth;
    int mode;

    public:

    using SystemCallParameterInterfaceType = AccessSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    AccessSystemCallParameters(std::string path, int modei)
        : SystemCallParameterInterfaceType(), pth(path), mode(modei) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 1) {
            param = mode;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string param) {
        if(parameter_index == 0) {
            param = pth;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using AccessSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class AccessSystemCall : public virtual AccessSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = AccessSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    AccessSystemCall() : SystemCallInterfaceType() {}

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