//
// SysCallWrite.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLLINK_H__
#define __SYSTEMCALLLINK_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using LinkSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 1025>;

template<typename RiscvArchType=Riscv32>
class LinkSystemCallParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const std::string oldpth, newpth;

    public:

    using SystemCallParameterInterfaceType = LinkSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    LinkSystemCallParameters(const std::string old_pth, const std::string new_pth)
        : SystemCallParameterInterfaceType(), oldpth(old_pth), newpth(new_pth) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 0) {
            param = oldpth;
            return true;
        }
        else if(parameter_index == 1) {
            param = newpth;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class LinkSystemCall : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LinkSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    LinkSystemCall() : SystemCallInterfaceType() {}

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