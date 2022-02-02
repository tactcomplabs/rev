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
using Prlimit64SystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class Prlimit64SystemCallParameters : public virtual Prlimit64SystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    pid_t pid; 
    int resource;
    const rlimit *new_limit;
    rlimit *old_limit;

    public:

    using SystemCallParameterInterfaceType = Prlimit64SystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    Prlimit64SystemCallParameters(pid_t pidp, int resourcep, rlimit * newlimit, rlimit * oldlimit)
        : SystemCallParameterInterfaceType(), pid(pidp), resource(resourcep), new_limit(newlimit), old_limit(oldlimit) {}

    size_t count() override { return 6UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_ptr & param) {
        if(parameter_index == 2) {
            param = old_limit;
            return true;
        }
        else if(parameter_index == 3) {
            param = new_limit;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 0) {
            param = pid;
            return true;
        }
        else if(parameter_index == 1) {
            param = resource;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using Prlimit64SystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class Prlimit64SystemCall : public virtual Prlimit64SystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = Prlimit64SystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    Prlimit64SystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, void_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF