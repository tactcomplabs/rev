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
using CloseSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 57>;

template<typename RiscvArchType=Riscv32>
class CloseSystemCallParameters : public virtual CloseSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;

    public:

    using SystemCallParameterInterfaceType = CloseSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    CloseSystemCallParameters(const int fd_i) : SystemCallParameterInterfaceType(), fd(fd_i) {}

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
using CloseSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 57>;

template<typename RiscvArchType=Riscv32>
class CloseSystemCall : public virtual CloseSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = CloseSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    private:

    void invoke_impl(SystemCallParameterInterfaceType & parameters, void_t & value, bool & invoc_success);

    public:

    CloseSystemCall() : SystemCallInterfaceType() {}

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