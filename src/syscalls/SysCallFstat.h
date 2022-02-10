//
// SysCallFstat.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFSTAT_H__
#define __SYSTEMCALLFSTAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

namespace SST { namespace RevCPU {

using stat_t = struct stat;

template<typename RiscvArchType=Riscv32>
using FstatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 79>;

template<typename RiscvArchType=Riscv32>
class FstatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    stat_t * buf;

    public:

    FstatParameters(int fildesi, stat_t * bufi)
        : fildes(fildesi), buf(bufi) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Fstat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FstatInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Fstat() : SystemCallInterfaceType() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
