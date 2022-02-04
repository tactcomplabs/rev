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
#ifndef __SYSTEMCALLLSEEK_H__
#define __SYSTEMCALLLSEEK_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using LseekSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 1039>;

template<typename RiscvArchType=Riscv32>
class LseekSystemCallParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    off_t offset;
    int whence;

    public:

    using SystemCallParameterInterfaceType = LseekSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    LseekSystemCallParameters(const int fd_i, const off_t offset_i, const int whence_i)
        : SystemCallParameterInterfaceType(), fd(fd_i), offset(offset_i), whence(whence_i) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }
        else if(parameter_index == 2) {
            param = whence;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, off_t& param) {
        if(parameter_index == 1) {
            param = offset;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class LseekSystemCall : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LseekSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    LseekSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, off_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF