//
// SysCallMadvise.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMADVISE_H__
#define __SYSTEMCALLMADVISE_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <sys/mman.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MadviseInterfaceType = SystemCallInterface<RiscvArchType, 233>;

template<typename RiscvArchType=Riscv32>
class MadviseParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * addr;
    size_t length;
    int advice;

    public:

    using SystemCallParameterInterfaceType = MadviseParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    MadviseParameters(void * addr_, size_t length_, int advice_)
        : SystemCallParameterInterfaceType(), addr(addr_), length(length_), advice(advice_) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = length;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 2) {
            param = advice;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, void * & param) {
        if(parameter_index == 0) {
            param = addr;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Madvise : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MadviseInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Madvise() : SystemCallInterfaceType() {}

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