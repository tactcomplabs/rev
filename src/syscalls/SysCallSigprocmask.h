//
// SysCallSigprocmask.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSIGPROCMASK_H__
#define __SYSTEMCALLSIGPROCMASK_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SigprocmaskInterfaceType = SystemCallInterfaceCode<RiscvArchType, 135>;

template<typename RiscvArchType=Riscv32>
class SigprocmaskParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int how;
    sigset_t * set;
    sigset_t * oset;

    public:

    SigprocmaskParameters(int howp, sigset_t * setp, sigset_t * osetp)
        : how(howp), set(setp), oset(osetp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Sigprocmask : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SigprocmaskInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Sigprocmask() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
