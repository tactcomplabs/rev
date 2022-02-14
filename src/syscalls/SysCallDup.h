//
// SysCallDup.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLDUP_H__
#define __SYSTEMCALLDUP_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using DupInterfaceType = SystemCallInterfaceCode<RiscvArchType, 23>;

template<typename RiscvArchType=Riscv32>
class DupParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;

    public:

    DupParameters(const int fildesp)
        : fildes(fildesp) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Dup : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = DupInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Dup() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
