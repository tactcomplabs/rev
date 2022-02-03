//
// SysCallFtruncate.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFTRUNCATE_H__
#define __SYSTEMCALLFTRUNCATE_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using FtruncateSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 46>;

template<typename RiscvArchType=Riscv32>
class FtruncateSystemCallParameters : public virtual FtruncateSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fildes;
    offset_t length;

    public:

    using SystemCallParameterInterfaceType = FtruncateSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    FtruncateSystemCallParameters(int fildesp, offset_t lengthp)
        : SystemCallParameterInterfaceType(), fildes(fildesp), length(lengthp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fildes;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, offset_t& param) {
        if(parameter_index == 1) {
            param = length;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using FtruncateSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 46>;

template<typename RiscvArchType=Riscv32>
class FtruncateSystemCall : public virtual FtruncateSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FtruncateSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    FtruncateSystemCall() : SystemCallInterfaceType() {}

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