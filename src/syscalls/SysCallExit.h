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

    ExitSystemCallParameters() : SystemCallParameterInterface() {}

    size_t count();

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<bool IsRiscv32>
class ExitSystemCall : public virtual SystemCallInterface<IsRiscv32> {

    using RiscvModeIntegerType = typename std::conditional<IsRiscv32, std::uint32_t, std::uint64_t>::type;

    public:

    ExitSystemCall() {}

    RiscvModeIntegerType code();
    
    template<typename ReturnType, typename SystemCallParameterType>
    bool invoke(SystemCallParameterType & parameters, ReturnType & value);

};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF