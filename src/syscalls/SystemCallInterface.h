//
// SystemCallInterface.h
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
#include <type_traits>

/*
 * void_t
 *
 * tag-type representing return type for system calls that return void
 */
struct void_t {};

namespace SST { namespace RevCPU {

class SystemCallParameterInterface {
    
    public:
    
    SystemCallParameterInterface() {}

    virtual size_t count() {
        return -1;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param) {
        return false;
    }
};

template<bool IsRiscv32>
class SystemCallInterface {

    using RiscvModeIntegerType = typename std::conditional<IsRiscv32, std::uint32_t, std::uint64_t>::type;

    public:

    SystemCallInterface() {}

    virtual RiscvModeIntegerType code() {
        return -1;
    }
    
    template<typename ReturnType>
    bool invoke(SystemCallParameterInterface & parameters, ReturnType & value) {
        return false;
    }
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF