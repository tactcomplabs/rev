//
// SysCallReadLinkAt.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLREADLINKAT_H__
#define __SYSTEMCALLREADLINKAT_H__

#include "SystemCallInterface.h"
#include <string>
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using ReadlinkatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 78>;

template<typename RiscvArchType=Riscv32>
class ReadlinkatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    std::string path;
    char * buf;
    size_t bufsize;

    public:

    ReadlinkatParameters(std::string pathp, char * bufp, size_t bufsz)
        : path(pathp), buf(bufp), bufsize(bufsz) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Readlinkat : public virtual SystemCallInterface<RiscvArchType> {
    
    public:

    using SystemCallInterfaceType = ReadlinkatInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;
    
    Readlinkat() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
