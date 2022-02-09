//
// SysCallFtruncate.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFTRUNCATE_H__
#define __SYSTEMCALLFTRUNCATE_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using FtruncateInterfaceType = SystemCallInterfaceCode<RiscvArchType, 46>;

template<typename RiscvArchType=Riscv32>
class FtruncateParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    off_t length;

    public:

    FtruncateParameters(int fildesp, off_t lengthp)
        : fildes(fildesp), length(lengthp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Ftruncate : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FtruncateInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Ftruncate() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
