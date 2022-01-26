//
// _PanExec_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLINTERFACE_H__
#define __SYSTEMCALLINTERFACE_H__

#include <cstddef>
#include <cstdint>
#include <optional>

namespace SST { namespace RevCPU {

/*
    SystemCallParameterInterface

    provides abstraction for all system call
    parameters

    Methods

    size_t count()

    users can query the abstraction for a parameter
    count

    std::optional<ParameterType> get(const size_t parameter_index);

    Users can 'get' a parameter by the linear index
    of the value in the system call interface; this
    method returns an 'optional' which when populated
    stores the requested return type (ParameterType).
    if the type of the parameter (ParameterType) the
    user requested is not equal to the type of the
    parameter at 'parameter_index' then the optional
    returns 'empty'.
 */
class SystemCallParameterInterface {
    
    virtual size_t count() {
        return -1;
    }

    template<typename ParameterType>
    std::optional<ParameterType> get(const size_t parameter_index) {
        std::nullopt;
    }
};

/*
    SystemCallInterface

    provides abstraction for all system calls

    Methods

    size_t count()

    users can query the abstraction for a parameter
    count

    std::optional<ParameterType> get(const size_t parameter_index);

    Users can 'get' a parameter by the linear index
    of the value in the system call interface; this
    method returns an 'optional' which when populated
    stores the requested return type (ParameterType).
    if the type of the parameter (ParameterType) the
    user requested is not equal to the type of the
    parameter at 'parameter_index' then the optional
    returns 'empty'.
 */
template<bool IsRiscv32>
class SystemCallInterface {

    using RiscvModeIntegerType = std::conditional<IsRiscv32, std::uint32_t, std::uint64_t>::type;

    virtual RiscvModeIntegerType code() {
        return -1;
    }
    
    template<typename ReturnType>
    std::optional<ReturnType> invoke(const SystemCallParameterInterface& parameters) {
        return std::nullopt;
    }
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF