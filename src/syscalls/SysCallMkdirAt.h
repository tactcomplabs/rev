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
using MkdirAtSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class MkdirAtSystemCallParameters : public virtual MkdirAtSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    std::string pth;
    size_t bcount;

    public:

    using SystemCallParameterInterfaceType = MkdirAtSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    MkdirAtSystemCallParameters(int fd_i, std::string path, size_t count_i)
        : SystemCallParameterInterfaceType(), fd(fd_i), pth(path), bcount(count_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 1) {
            param = pth;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t& param) {
        if(parameter_index == 2) {
            param = bcount;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using MkdirAtSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class MkdirAtSystemCall : public virtual MkdirAtSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MkdirAtSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    MkdirAtSystemCall() : SystemCallInterfaceType() {}

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