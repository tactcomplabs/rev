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
#ifndef __SYSTEMCALLEXIT_H__
#define __SYSTEMCALLEXIT_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

class ExitSystemCallParameters : public virtual SystemCallParameterInterface {
    
    int status;

    public:

    ExitSystemCallParameters(const int stat) : SystemCallParameterInterface(), status(stat) {}

    size_t count() override;

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class ExitSystemCall : public virtual SystemCallInterface<RiscvArchType> {

    using RiscvModeIntegerType = typename SystemCallInterface<RiscvArchType>::RiscvModeIntegerType;
    
    public:

    const static RiscvModeIntegerType code_value = static_cast<RiscvModeIntegerType>(93);

    ExitSystemCall() {}

    RiscvModeIntegerType code() override;
    
    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterface & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterface & parameters, void_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF