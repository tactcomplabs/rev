//
// SysCallChdir.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLCHDIR_H__
#define __SYSTEMCALLCHDIR_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using ChdirInterfaceType = SystemCallInterface<RiscvArchType, 49>;

template<typename RiscvArchType=Riscv32>
class ChdirParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    std::string pth;

    public:

    using SystemCallParameterInterfaceType = ChdirParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    ChdirParameters(std::string path)
        : SystemCallParameterInterfaceType(), pth(path) {}

    size_t count() override { return 1UL; }

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
};

template<typename RiscvArchType=Riscv32>
class Chdir : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = ChdirInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Chdir() : SystemCallInterfaceType() {}

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