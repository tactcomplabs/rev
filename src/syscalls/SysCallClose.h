//
// SysCallExit.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLCLOSE_H__
#define __SYSTEMCALLCLOSE_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using CloseInterfaceType = SystemCallInterface<RiscvArchType, 57>;

template<typename RiscvArchType=Riscv32>
class CloseParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;

    public:

    using SystemCallParameterInterfaceType = CloseParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    CloseParameters(const int fd_i) : SystemCallParameterInterfaceType(), fd(fd_i) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Close : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = CloseInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Close() : SystemCallInterfaceType() {}

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