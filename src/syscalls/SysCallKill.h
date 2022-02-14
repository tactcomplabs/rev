//
// SysCallKill.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLKILL_H__
#define __SYSTEMCALLKILL_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using KillInterfaceType = SystemCallInterfaceCode<RiscvArchType, 129>;

template<typename RiscvArchType=Riscv32>
class KillParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    pid_t pid;
    int sig;

    public:

    KillParameters(const pid_t pid_i, const int sig_i) : pid(pid_i), sig(sig_i) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Kill : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = KillInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Kill() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
