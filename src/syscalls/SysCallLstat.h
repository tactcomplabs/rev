//
// SysCallStat.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLLSTAT_H__
#define __SYSTEMCALLLSTAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>

#include <unistd.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

using stat_t = struct stat;

template<typename RiscvArchType=Riscv32>
using LstatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1039>;

template<typename RiscvArchType=Riscv32>
class LstatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    std::string pth;
    stat_t * buf;

    public:

    LstatParameters(std::string path, stat_t * bufi)
        : pth(path), buf(bufi) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Lstat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LstatInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Lstat() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
