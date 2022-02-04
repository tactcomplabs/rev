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

template<typename RiscvArchType=Riscv32>
using TgkillParametersInterfaceType = SystemCallInterface<RiscvArchType, 131>;

template<typename RiscvArchType=Riscv32>
class TgkillParameters : public virtual TgkillParametersInterfaceType<RiscvArchType> {
    
    private:

    int tgid;
    int tid;
    int sig;

    public:

    using SystemCallParameterInterfaceType = TgkillParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    TgkillParameters(const int tgid_i, const int tid_i, const int sig_i) : SystemCallParameterInterfaceType(), tgid(tgid_i), tid(tid_i), sig(sig_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = tgid;
            return true;
        }
        else if(parameter_index == 1) {
            param = tid;
            return true;
        }
        else if(parameter_index == 2) {
            param = sig;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using TgkillInterfaceType = SystemCallInterface<RiscvArchType, 131>;

template<typename RiscvArchType=Riscv32>
class Tgkill : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TgkillInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Tgkill() : SystemCallInterfaceType() {}

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