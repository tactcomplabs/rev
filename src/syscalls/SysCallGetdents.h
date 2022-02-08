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
using GetdentsInterfaceType = SystemCallInterfaceCode<RiscvArchType, 61>;

template<typename RiscvArchType=Riscv32>
class GetdentsParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    void *dirp;
    size_t count_;
    
    public:

    GetdentsParameters(int fdp, void *dirpp, size_t countp)
        : fd(fdp), dirp(dirpp), count_(countp) {}

    size_t count() override { return 2UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);  
};

template<typename RiscvArchType=Riscv32>
class Getdents : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = GetdentsInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Getdents() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
