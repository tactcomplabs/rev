//
// SysCallMkdirAt.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMKDIRAT_H__
#define __SYSTEMCALLMKDIRAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MkdiratInterfaceType = SystemCallInterfaceCode<RiscvArchType, 34>;

template<typename RiscvArchType=Riscv32>
class MkdiratParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    std::string pth;
    size_t bcount;

    public:

    MkdiratParameters(int fd_i, std::string path, size_t count_i)
        : fd(fd_i), pth(path), bcount(count_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Mkdirat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MkdiratInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Mkdirat() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
