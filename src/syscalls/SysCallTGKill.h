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
using TgkillInterfaceType = SystemCallInterfaceCode<RiscvArchType, 131>;

template<typename RiscvArchType=Riscv32>
class TgkillParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int tgid;
    int tid;
    int sig;

    public:

    TgkillParameters(const int tgid_i, const int tid_i, const int sig_i)
        : tgid(tgid_i), tid(tid_i), sig(sig_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Tgkill : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = TgkillInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Tgkill() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
