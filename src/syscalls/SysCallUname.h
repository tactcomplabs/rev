//
// SysCallUname.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLUNAME_H__
#define __SYSTEMCALLUNAME_H__

#include "SystemCallInterface.h"
#include <sys/utsname.h>
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using UnameSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class UnameSystemCallParameters : public virtual UnameSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    utsname * name;

    public:

    using SystemCallParameterInterfaceType = UnameSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    UnameSystemCallParameters(utsname * buf)
        : SystemCallParameterInterfaceType(), name(buf) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, utsname * & param) {
        if(parameter_index == 0) {
            param = name;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using UnameSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class UnameSystemCall : public virtual UnameSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = UnameSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    UnameSystemCall() : SystemCallInterfaceType() {}

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