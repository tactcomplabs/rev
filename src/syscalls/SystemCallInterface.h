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

// how to support uint128_t? that's not supported in c++11 compilers...
//
// https://stackoverflow.com/questions/34588650/uint128-t-does-not-name-a-type
//
typedef unsigned int uint128_t __attribute__((mode(TI)));

using cvoid_ptr = const void *;
using void_ptr = void *;

/*
 * void_t
 *
 * tag-type representing return type for system calls that return void
 */
struct void_t {};

namespace SST { namespace RevCPU {

// type tags for each riscv architecture supported in REV
//
struct RiscvArch {
    using int_type = std::uint32_t;
};

struct Riscv32 : RiscvArch {
    using int_type = std::uint32_t;
};

struct Riscv64 : RiscvArch {
    using int_type = std::uint64_t;
};

struct Riscv128 : RiscvArch {
    using int_type = uint128_t;
};

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

template<typename RiscvArchType=Riscv32>
class SystemCallInterface {

    protected:

    bool success;

    public:

    using IsRiscv32 = typename std::conditional< std::is_same<RiscvArchType, Riscv32>::value, std::true_type, std::false_type>::type;
    using IsRiscv64 = typename std::conditional< std::is_same<RiscvArchType, Riscv64>::value, std::true_type, std::false_type>::type;
    using IsRiscv128 = typename std::conditional< std::is_same<RiscvArchType, Riscv128>::value, std::true_type, std::false_type>::type;

    using RiscvModeIntegerType = typename std::conditional< IsRiscv32::value,
        Riscv32::int_type, // TRUE
        typename std::conditional< IsRiscv64::value, // FALSE
                Riscv32::int_type, // TRUE
                typename std::conditional< IsRiscv128::value, // FALSE
                        Riscv128::int_type, // TRUE
                        Riscv32::int_type // FALSE - we fall back on Riscv32
                >::type
            >::type
        >::type;


    SystemCallInterface() : success(false) {}

    bool invoke_success() {
        return success;
    }

    virtual RiscvModeIntegerType code() {
        return -1;
    }
    
    template<typename ReturnType>
    void invoke(SystemCallParameterInterface & parameters, ReturnType & value) {
        success = false;
    }
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF