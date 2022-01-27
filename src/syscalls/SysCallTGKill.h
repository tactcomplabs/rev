//
// SysCallTGKill.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLTGKILL_H__
#define __SYSTEMCALLTGKILL_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

class TGKillSystemCallParameters : public virtual SystemCallParameterInterface {

    private:

    // pid_t is type equivalent to int
    //
    pid_t pid;
    int sig;

    public:

    TGKillSystemCallParameters(const pid_t pid_i, const int sig_i) : SystemCallParameterInterface(), pid(pid_i), sig(sig_i) {}

    size_t count() override;

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class TGKillSystemCall : public virtual SystemCallInterface<RiscvArchType> {

    using RiscvModeIntegerType = typename SystemCallInterface<RiscvArchType>::RiscvModeIntegerType;

    public:

    const static RiscvModeIntegerType code_value = static_cast<RiscvModeIntegerType>(131);

    TGKillSystemCall() {}

    RiscvModeIntegerType code() override;
    
    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterface & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterface & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF