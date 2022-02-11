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
using LseekInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1039>;

template<typename RiscvArchType=Riscv32>
class LseekParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    off_t offset;
    int whence;

    public:

    LseekParameters(const int fd_i, const off_t offset_i, const int whence_i)
        : fd(fd_i), offset(offset_i), whence(whence_i) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Lseek : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LseekInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Lseek() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
