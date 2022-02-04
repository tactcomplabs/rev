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
using ReadlinkatParametersInterfaceType = SystemCallInterface<RiscvArchType, 78>;

template<typename RiscvArchType=Riscv32>
class ReadlinkatParameters : public virtual ReadlinkatParametersInterfaceType<RiscvArchType> {
    
    private:

    std::string path;
    char * buf;
    size_t bufsize;

    public:

    using SystemCallParameterInterfaceType = ReadlinkatParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    ReadlinkatParameters(std::string pathp, char * bufp, size_t bufsz)
        : SystemCallParameterInterfaceType(), path(pathp), buf(bufp), bufsize(bufsz) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 0) {
            param = path;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, char* & param) {
        if(parameter_index == 1) {
            param = buf;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t& param) {
        if(parameter_index == 2) {
            param = bufsize;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using ReadlinkatInterfaceType = SystemCallInterface<RiscvArchType, 78>;

template<typename RiscvArchType=Riscv32>
class Readlinkat : public virtual SystemCallInterface<RiscvArchType> {
    
    public:

    using SystemCallInterfaceType = ReadlinkatInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;
    
    Readlinkat() : SystemCallInterfaceType() {}

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