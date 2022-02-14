//
// SysCallWrite.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLLINK_H__
#define __SYSTEMCALLLINK_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using LinkInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1025>;

template<typename RiscvArchType=Riscv32>
class LinkParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const std::string oldpth, newpth;

    public:

    LinkParameters(const std::string old_pth, const std::string new_pth)
        : oldpth(old_pth), newpth(new_pth) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Link : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LinkInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Link() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
