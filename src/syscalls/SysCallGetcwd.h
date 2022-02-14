//
// SysCallGetcwd.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETCWD_H__
#define __SYSTEMCALLGETCWD_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetcwdInterfaceType = SystemCallInterfaceCode<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class GetcwdParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const std::string pth;
    const size_t size;

    public:

    GetcwdParameters(std::string path, const size_t sz)
        : pth(path), size(sz) {}

    GetcwdParameters(std::string path)
        : pth(path), size(-1) {}


    size_t count() override {
        return (size == -1) ? 1UL : 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Getcwd : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetcwdInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Getcwd() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
