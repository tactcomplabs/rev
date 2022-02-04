//
// SysCallFcntl.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFCNTL_H__
#define __SYSTEMCALLFCNTL_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using FcntlInterfaceType = SystemCallInterface<RiscvArchType, 25>;

template<typename RiscvArchType=Riscv32>
class FcntlParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    int cmd;

    public:

    using SystemCallParameterInterfaceType = FcntlCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    FcntlParameters(int fildesp, int cmdp)
        : SystemCallParameterInterfaceType(), fildes(fildesp), cmd(cmdp) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 0) {
            param = fildes;
            return true;
        }
        else if(parameter_index == 1) {
            param = cmd;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Fcntl : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FcntlInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Fcntl() : SystemCallInterfaceType() {}

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