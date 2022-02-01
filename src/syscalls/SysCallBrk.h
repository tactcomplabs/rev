//
// SysCallBrk.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLBRK_H__
#define __SYSTEMCALLBRK_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using BrkSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 214>;

template<typename RiscvArchType=Riscv32>
class BrkSystemCallParameters : public virtual BrkSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    cvoid_ptr addr;

    public:

    using SystemCallParameterInterfaceType = BrkSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    BrkSystemCallParameters(const cvoid_ptr addr_i) : SystemCallParameterInterfaceType(), addr(addr_i) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = addr;
            return true;
        }

        return false;
    }
};


template<typename RiscvArchType=Riscv32>
using BrkSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 214>;

template<typename RiscvArchType=Riscv32>
class BrkSystemCall : public virtual BrkSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = BrkSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    BrkSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, void_ptr & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF