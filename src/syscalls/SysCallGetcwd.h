//
// SysCallGetcwd.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETCWD_H__
#define __SYSTEMCALLGETCWD_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetcwdSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class GetcwdSystemCallParameters : public virtual GetcwdSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    const std::string pth;
    const size_t size;

    public:

    using SystemCallParameterInterfaceType = GetcwdSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetcwdSystemCallParameters(std::string path, const size_t sz)
        : SystemCallParameterInterfaceType(), pth(path), size(sz) {}

    GetcwdSystemCallParameters(std::string path)
        : SystemCallParameterInterfaceType(), pth(path), size(-1) {}


    size_t count() override {
        return (size == -1) ? 1UL : 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 0) {
            param = pth;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t& param) {
        if(parameter_index == 0) {
            param = size;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using GetcwdSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class GetcwdSystemCall : public virtual GetcwdSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetcwdSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    GetcwdSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, std::string & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF