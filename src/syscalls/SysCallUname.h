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
using UnameInterfaceType = SystemCallInterface<RiscvArchType, 160>;

template<typename RiscvArchType=Riscv32>
class UnameParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    utsname * name;

    public:

    using SystemCallParameterInterfaceType = UnameParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    UnameParameters(utsname * buf)
        : SystemCallParameterInterfaceType(), name(buf) {}

    size_t count() override { return 1UL; }

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
class Uname : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = UnameInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;    

    Uname() : SystemCallInterfaceType() {}

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