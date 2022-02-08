//
// SysCallPrlimit64.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLPRLIMIT64_H__
#define __SYSTEMCALLPRLIMIT64_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/time.h> 
#include <sys/resource.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using Prlimit64InterfaceType = SystemCallInterfaceCode<RiscvArchType, 261>;

template<typename RiscvArchType=Riscv32>
class Prlimit64Parameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:
    
    pid_t pid; 
    int resource;
    const rlimit *new_limit;
    rlimit *old_limit;

    public:

    Prlimit64Parameters(pid_t pidp, int resourcep, rlimit * newlimit, rlimit * oldlimit)
        : pid(pidp), resource(resourcep), new_limit(newlimit), old_limit(oldlimit) {}

    size_t count() override { return 4UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Prlimit64 : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = Prlimit64InterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    public:

    Prlimit64() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
