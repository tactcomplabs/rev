//
// SysCallRenameAt.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLRENAMEAT_H__
#define __SYSTEMCALLRENAMEAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <unistd.h>

//     int
//     renameat(int fromfd, const char *from, int tofd, const char *to);


namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using RenameatInterfaceType = SystemCallInterfaceCode<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class RenameatParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    const int fromfd;
    const std::string from;
    const int tofd;
    const std::string to;

    public:

    RenameatParameters(int frfd, const std::string fromstr, int tfd, const std::string to_)
        : fromfd(frfd), from(fromstr), tofd(tfd), to(to_) {}

    size_t count() override { return 4UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Renameat : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = RenameatInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Renameat() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
