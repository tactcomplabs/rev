//
// SysCallDup.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLDUP_H__
#define __SYSTEMCALLDUP_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using DupSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class DupSystemCallParameters : public virtual DupSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fildes;

    public:

    using SystemCallParameterInterfaceType = DupSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    DupSystemCallParameters(const int fildesp)
        : SystemCallParameterInterfaceType(), fildes(fildesp) {}

    size_t count() override { return 1UL; }

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
};

template<typename RiscvArchType=Riscv32>
using DupSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class DupSystemCall : public virtual DupSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = DupSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    DupSystemCall() : SystemCallInterfaceType() {}

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