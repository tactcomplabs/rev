//
// SysCallUnlink.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLUNLINK_H__
#define __SYSTEMCALLUNLINK_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using UnlinkParametersInterfaceType = SystemCallInterface<RiscvArchType, 1026>;

template<typename RiscvArchType=Riscv32>
class UnlinkParameters : public virtual UnlinkParametersInterfaceType<RiscvArchType> {
    
    private:

    const std::string pth;

    public:

    using SystemCallParameterInterfaceType = UnlinkParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    UnlinkParameters(const std::string path)
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
using UnlinkInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1026>;

template<typename RiscvArchType=Riscv32>
class Unlink : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = UnlinkInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;    
    
    Unlink() : SystemCallInterfaceType() {}

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