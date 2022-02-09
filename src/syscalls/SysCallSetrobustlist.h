//
// SysCallSetrobustlist.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSETROBUSTLIST_H__
#define __SYSTEMCALLSETROBUSTLIST_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>
#include <linux/futex.h>
#include <syscall.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SetrobustlistInterfaceType = SystemCallInterfaceCode<RiscvArchType, 99>;

template<typename RiscvArchType=Riscv32>
class SetrobustlistParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    robust_list_head * hptr;
    size_t len;

    public:

    SetrobustlistParameters(robust_list_head * ptr, size_t lenp)
        : hptr(ptr), len(lenp) {}

    size_t count() override {
        return 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Setrobustlist : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SetrobustlistInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Setrobustlist() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
