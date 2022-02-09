//
// SysCallAccess.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLACCESS_H__
#define __SYSTEMCALLACCESS_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

using stat_t = struct stat;

template<typename RiscvArchType=Riscv32>
using AccessInterfaceType = SystemCallInterfaceCode<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class AccessParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    std::string pth;
    int mode;

    public:

    AccessParameters(std::string path, int modei)
        : pth(path), mode(modei) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Access : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = AccessInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Access() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
