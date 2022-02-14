//
// SysCallDup3.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLDUP3_H__
#define __SYSTEMCALLDUP3_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using Dup3InterfaceType = SystemCallInterfaceCode<RiscvArchType, 24>;

template<typename RiscvArchType=Riscv32>
class Dup3Parameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int oldfd, newfd, flags;

    public:

    Dup3Parameters(const int ofdp, const int nfdp, const int flagsp)
        : oldfd(ofdp), newfd(nfdp), flags(flagsp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Dup3 : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = Dup3InterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Dup3() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
