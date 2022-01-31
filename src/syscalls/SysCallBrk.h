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
#ifndef __SYSTEMCALLBRK_H__
#define __SYSTEMCALLBRK_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

class BrkSystemCallParameters : public virtual SystemCallParameterInterface {
    
    cvoid_ptr addr;

    public:

    BrkSystemCallParameters(cvoid_ptr addr_) : SystemCallParameterInterface(), addr(addr_) {}

    size_t count() override;

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class BrkSystemCall : public virtual SystemCallInterface<RiscvArchType> {

    using RiscvModeIntegerType = typename SystemCallInterface<RiscvArchType>::RiscvModeIntegerType;
    
    public:

    const static RiscvModeIntegerType code_value = static_cast<RiscvModeIntegerType>(214);

    BrkSystemCall() {}

    RiscvModeIntegerType code() override;
    
    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterface & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterface & parameters, void_ptr & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF