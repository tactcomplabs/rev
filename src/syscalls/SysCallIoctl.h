//
// SysCallIoctl.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLIOCTL_H__
#define __SYSTEMCALLIOCTL_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using IoctlInterfaceType = SystemCallInterface<RiscvArchType, 29>;


template<typename RiscvArchType=Riscv32>
class IoctlParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    unsigned long request;

    public:

    using SystemCallParameterInterfaceType = IoctlParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;
    
    IoctlParameters(int fildesp, unsigned long requestp)
        : SystemCallParameterInterfaceType(), fildes(fildesp), request(requestp) {}

    size_t count() override { return 2UL; }

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
    bool get(const size_t parameter_index, unsigned long& param) {
        if(parameter_index == 1) {
            param = request;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Ioctl : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = IoctlInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Ioctl() : SystemCallInterfaceType() {}

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