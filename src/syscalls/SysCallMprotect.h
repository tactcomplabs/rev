//
// SysCallMprotect.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMPROTECT_H__
#define __SYSTEMCALLMPROTECT_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MprotectSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 226>;

template<typename RiscvArchType=Riscv32>
class MprotectSystemCallParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    void * addr;
    size_t len;
    int prot;

    public:

    using SystemCallParameterInterfaceType = MprotectSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    MprotectSystemCallParameters(void * addrp, size_t lenp, int protp)
        : SystemCallParameterInterfaceType(), addr(addrp), len(lenp), prot(protp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_ptr & param) {
        if(parameter_index == 0) {
            param = addr;
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

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 3) {
            param = prot;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
class MprotectSystemCall : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MprotectSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    MprotectSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, void_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF