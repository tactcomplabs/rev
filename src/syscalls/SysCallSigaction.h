//
// SysCallSigaction.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSIGACTION_H__
#define __SYSTEMCALLSIGACTION_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <signal.h>

namespace SST { namespace RevCPU {

using sigaction_t = struct sigaction;

template<typename RiscvArchType=Riscv32>
using SigactionInterfaceType = SystemCallInterfaceCode<RiscvArchType, 134>;

template<typename RiscvArchType=Riscv32>
class SigactionParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int sig;
    sigaction_t * act;
    sigaction_t * oact;

    public:

    SigactionParameters(int sigp, sigaction_t * actp, sigaction_t * oactp)
        : sig(sigp), act(actp), oact(oactp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Sigaction : public virtual SigactionInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SystemCallInterface<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Sigaction() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
