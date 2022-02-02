//
// SysCallGetdents.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLGETDENTS_H__
#define __SYSTEMCALLGETDENTS_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <dirent.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using GetdentsCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class GetdentsSystemCallParameters : public virtual GetdentsCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    void *dirp;
    size_t count;
    
    public:

    using SystemCallParameterInterfaceType = GetdentsCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    GetdentsSystemCallParameters(int fdp, void *dirpp, size_t countp)
        : SystemCallParameterInterfaceType(), fd(fdp), dirp(dirpp), count(countp) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, void* & param) {
        if(parameter_index == 0) {
            param = dirp;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 0) {
            param = count;
            return true;
        }
        
        return false;
    }        
};

template<typename RiscvArchType=Riscv32>
using GetdentsSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class GetdentsSystemCall : public virtual GetdentsSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetdentsSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    GetdentsSystemCall() : SystemCallInterfaceType() {}

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