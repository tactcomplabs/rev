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
#ifndef __SYSTEMCALLLINKAT_H__
#define __SYSTEMCALLLINKAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using LinkatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 37>;

template<typename RiscvArchType=Riscv32>
class LinkatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const int fd1, fd2, flag;
    const std::string oldpth;
    const std::string newpth;

    public:

    LinkatParameters(const int fd_1, const std::string old_pth, const int fd_2, const std::string new_pth, const int flag_i)
        : fd1(fd_1), oldpth(old_pth), fd2(fd_2), newpth(new_pth), flag(flag_i) {}

    size_t count() override { return 5UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Linkat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LinkatInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Linkat() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
