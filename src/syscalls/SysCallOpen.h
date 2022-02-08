//
// SysCallOpen.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLOPEN_H__
#define __SYSTEMCALLOPEN_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>

#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using OpenInterfaceType = SystemCallInterfaceCode<RiscvArchType, 1024>;

template<typename RiscvArchType=Riscv32>
class OpenParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    std::string path;
    int oflag;
    mode_t mode;

    public:

    OpenParameters(std::string path_i, int oflag_i, mode_t mode_i)
        : path(path_i), oflag(oflag_i), mode(mode_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Open : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = OpenInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Open() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
