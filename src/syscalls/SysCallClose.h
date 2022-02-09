//
// SysCallExit.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLCLOSE_H__
#define __SYSTEMCALLCLOSE_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using CloseInterfaceType = SystemCallInterfaceCode<RiscvArchType, 57>;

template<typename RiscvArchType=Riscv32>
class CloseParameters : public virtual SystemCallParameterInterface<RiscvArchType> {

    private:

    int fd;

    public:

    CloseParameters(int fdi)
        : fd(fdi) {}

    size_t count() override { return 1UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Close : public virtual SystemCallInterface<RiscvArchType> {

    public:

    using SystemCallInterfaceType = CloseInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;

    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Close() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
