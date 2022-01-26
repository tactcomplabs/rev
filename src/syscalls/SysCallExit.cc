//
// SysCallExit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallExit.h"
#include <algorithm>

namespace SST { namespace RevCPU {

size_t ExitSystemCallParameters::count() {
    return 1UL;
}

template<typename ParameterType>
std::optional<ParameterType> ExitSystemCallParameters::get(const size_t parameter_index) {
    if(parameter_index == 0) {
        if(std::conditional< std::is_same<ParameterType, int> >::value) {
            return std::make_optional<int>(status);
        }
    }

    return std::nullopt;
}

template<bool IsRiscv32>
ExitSystemCall<IsRiscv32>::RiscvModeIntegerType ExitSystemCall<IsRiscv32>::code() {
    return static_cast<RiscvModeIntegerType>(93);
}

template<bool IsRiscv32>
template<typename ReturnType>
std::optional<ReturnType> ExitSystemCall<IsRiscv32>::invoke(const SystemCallParameterInterface& parameters) {

    if(std::conditional< std::is_same<ReturnType, int> >::value) {
        return std::nullopt;
    }

    if(parameters.count() == 1) {
        const std::optional<int> status = parameters.get<int>(0);
        if(status.has_value()) {
            std::make_optional<int>(exit(*status));
        }
    }

    return std::nullopt;
}

} /* end namespace RevCPU */ } // end namespace SST