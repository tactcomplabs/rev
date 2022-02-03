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
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using ReadLinkAtSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 78>;

template<typename RiscvArchType=Riscv32>
class ReadLinkAtSystemCallParameters : public virtual ReadLinkAtSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    std::string path;
    char * buf;
    size_t bufsize;

    public:

    using SystemCallParameterInterfaceType = ReadLinkAtSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    ReadLinkAtSystemCallParameters(std::string pathp, char * bufp, size_t bufsz)
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
using ReadLinkAtSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 78>;

template<typename RiscvArchType=Riscv32>
class ReadLinkAtSystemCall : public virtual ReadLinkAtSystemCallInterfaceType<RiscvArchType> {
    
    public:

    using SystemCallInterfaceType = ReadLinkAtSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    
    
    ReadLinkAtSystemCall() : SystemCallInterfaceType() {}

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