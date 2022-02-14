//
// SysCallIoctl.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLIOCTL_H__
#define __SYSTEMCALLIOCTL_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using IoctlInterfaceType = SystemCallInterfaceCode<RiscvArchType, 29>;

template<typename RiscvArchType=Riscv32>
class IoctlParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    unsigned long request;

    public:

    IoctlParameters(int fildesp, unsigned long requestp)
        : fildes(fildesp), request(requestp) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Ioctl : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = IoctlInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Ioctl() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
