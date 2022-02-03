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

template<typename RiscvArchType=Riscv32>
using SigactionSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 134>;

template<typename RiscvArchType=Riscv32>
class SigactionSystemCallParameters : public virtual SigactionSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int sig;
    sigaction * act;
    sigaction * oact;

    public:

    using SystemCallParameterInterfaceType = SigactionSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    SigactionSystemCallParameters(int sigp, sigaction * actp, sigaction * oactp)
        : SystemCallParameterInterfaceType(), sig(sigp), act(actp), oact(oactp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = sig;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, sigaction * & param) {
        if(parameter_index == 1) {
            param = act;
            return true;
        }
        else if(parameter_index == 2) {
            param = oact;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using SigactionSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 134>;

template<typename RiscvArchType=Riscv32>
class SigactionSystemCall : public virtual SigactionSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = SigactionSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    SigactionSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF