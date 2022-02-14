//
// SysCallSettidaddress.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSETTIDADDRESS_H__
#define __SYSTEMCALLSETTIDADDRESS_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SettidaddressInterfaceType = SystemCallInterfaceCode<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class SettidaddressParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int* tidptr;

    public:

    SettidaddressParameters(int * tidptrp)
        : tidptr(tidptrp) {}

    size_t count() override {
        return 1UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Settidaddress : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SettidaddressInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Settidaddress() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
