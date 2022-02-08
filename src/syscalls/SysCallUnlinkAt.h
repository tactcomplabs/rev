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
#ifndef __SYSTEMCALLUNLINKAT_H__
#define __SYSTEMCALLUNLINKAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using UnlinkatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 35>;

template<typename RiscvArchType=Riscv32>
class UnlinkatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const int dirfd;
    const std::string pth;
    const int flags;

    public:

    UnlinkatParameters(const int dirfd_, cchar_ptr path, const size_t pathsz, const int flags_)
        : dirfd(dirfd_), pth(path), flags(flags_)  {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Unlinkat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = UnlinkatInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;    

    Unlinkat() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
