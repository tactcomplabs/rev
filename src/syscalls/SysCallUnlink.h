//
// SysCallUnlink.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLUNLINK_H__
#define __SYSTEMCALLUNLINK_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using UnlinkInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1026>;

template<typename RiscvArchType=Riscv32>
class UnlinkParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const std::string pth;

    public:

    UnlinkParameters(const std::string path)
        : pth(path) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Unlink : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = UnlinkInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;    
    
    Unlink() : SystemCallInterfaceType() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
