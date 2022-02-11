//
// SysCallOpenAt.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLOPENAT_H__
#define __SYSTEMCALLOPENAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <cstdlib>
#include <string>

#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using OpenatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 56>;

template<typename RiscvArchType=Riscv32>
class OpenatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    std::string path;
    int oflag;
    mode_t mode;

    public:

    OpenatParameters(int fd_i, std::string path_i, int oflag_i, mode_t mode_i)
        : fd(fd_i), path(path_i), oflag(oflag_i), mode(mode_i) {}

    size_t count() override { return 4UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Openat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = OpenatInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Openat() : SystemCallInterfaceType() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
