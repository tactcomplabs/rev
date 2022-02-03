//
// SysCallMkdir.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMKDIR_H__
#define __SYSTEMCALLMKDIR_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MkdirSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 1030>;

template<typename RiscvArchType=Riscv32>
class MkdirSystemCallParameters : public virtual MkdirSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    std::string pth;
    mode_t mode;

    public:

    using SystemCallParameterInterfaceType = MkdirSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    MkdirSystemCallParameters(std::string path, mode_t mde)
        : SystemCallParameterInterfaceType(), pth(path), mode(mde) {}

    size_t count() override { return 2UL; }

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
    bool get(const size_t parameter_index, mode_t& param) {
        if(parameter_index == 1) {
            param = mode;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using MkdirSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 1030>;

template<typename RiscvArchType=Riscv32>
class MkdirSystemCall : public virtual MkdirSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MkdirSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    MkdirSystemCall() : SystemCallInterfaceType() {}

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