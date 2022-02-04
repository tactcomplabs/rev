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
#include <string>

namespace SST { namespace RevCPU {

using stat_t = struct stat;

template<typename RiscvArchType=Riscv32>
using FstatInterfaceType = SystemCallInterface<RiscvArchType, 79>;

template<typename RiscvArchType=Riscv32>
class FstatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    stat_t * buf;

    public:

    using SystemCallParameterInterfaceType = FstatParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    FstatParameters(int fildesi, stat_t * bufi)
        : SystemCallParameterInterfaceType(), fildes(fildesi), buf(bufi) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fildes;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, stat * param) {
        if(parameter_index == 1) {
            param = buf;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
class Fstat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FstatInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Fstat() : SystemCallInterfaceType() {}

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