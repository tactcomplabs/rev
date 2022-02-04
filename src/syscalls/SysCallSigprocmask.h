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
#include <signal.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using SigprocmaskParametersInterfaceType = SystemCallInterface<RiscvArchType, 135>;

template<typename RiscvArchType=Riscv32>
class SigprocmaskParameters : public virtual SigprocmaskParametersInterfaceType<RiscvArchType> {
    
    private:

    int how;
    sigset_t * set;
    sigset_t * oset;

    public:

    using SystemCallParameterInterfaceType = SigprocmaskParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    SigprocmaskParameters(int howp, sigset_t * setp, sigset_t * osetp)
        : SystemCallParameterInterfaceType(), how(howp), set(setp), oset(osetp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 0) {
            param = tp;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, sigset_t * & param) {
        if(parameter_index == 1) {
            param = set;
            return true;
        }
        else if(parameter_index == 2) {
            param = oset;
            return true;
        }        
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using SigprocmaskInterfaceType = SystemCallInterface<RiscvArchType, 135>;

template<typename RiscvArchType=Riscv32>
class Sigprocmask : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SigprocmaskInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Sigprocmask() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, clock_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF