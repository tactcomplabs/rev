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
#include <unistd.h>

namespace SST { namespace RevCPU {

using stat_t = struct stat;

template<typename RiscvArchType=Riscv32>
using FstatatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 79>;

template<typename RiscvArchType=Riscv32>
class FstatatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    std::string path;
    stat_t * buf;
    int flag;

    public:

    FstatatParameters(int fdi, std::string pathi, stat_t * bufi, int flagi)
        : fd(fdi), path(pathi), buf(bufi), flag(flagi) {}

    size_t count() override {
        return 4;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Fstatat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FstatatInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Fstatat() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
