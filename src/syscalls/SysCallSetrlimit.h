//
// SysCallSetrlimit.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSETRLIMIT_H__
#define __SYSTEMCALLSETRLIMIT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SetrlimitInterfaceType = SystemCallInterfaceCode<RiscvArchType, 164>;

template<typename RiscvArchType=Riscv32>
class SetrlimitParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int resource;
    rlimit * rlp;

    public:

    SetrlimitParameters(int resourcep, rlimit * rlpp)
        : resource(resourcep), rlp(rlpp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Setrlimit : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SetrlimitInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Setrlimit() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
