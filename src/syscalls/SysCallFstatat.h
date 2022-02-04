//
// SysCallFstatat.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFSTATAT_H__
#define __SYSTEMCALLFSTATAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

using stat_t = struct stat;

template<typename RiscvArchType=Riscv32>
using FstatatInterfaceType = SystemCallInterface<RiscvArchType, 79>;

template<typename RiscvArchType=Riscv32>
class FstatatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    std::string path;
    stat_t * buf;
    int flag;

    public:

    using SystemCallParameterInterfaceType = FstatatParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    FstatatParameters(int fdi, std::string pathi, stat * bufi, int flagi)
        : SystemCallParameterInterfaceType(), fd(fdi), path(pathi), buf(bufi), flag(flagi) {}

    size_t count() override {
        return (size == -1) ? 1UL : 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }
        else if(parameter_index == 3) {
            param = flag;
            return true;
        }
        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 1) {
            param = path;
            return true;
        }

        return false;
    }    

    template<>
    bool get(const size_t parameter_index, stat_t * & param) {
        if(parameter_index == 2) {
            param = buf;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Fstatat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FstatatInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Fstatat() : SystemCallInterfaceType() {}

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