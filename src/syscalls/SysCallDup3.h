//
// SysCallDup3.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLDUP3_H__
#define __SYSTEMCALLDUP3_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using Dup3InterfaceType = SystemCallInterface<RiscvArchType, 24>;

template<typename RiscvArchType=Riscv32>
class Dup3Parameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int oldfd, newfd, flags;

    public:

    using SystemCallParameterInterfaceType = Dup3ParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    Dup3Parameters(const int ofdp, const int nfdp, const int flagsp)
        : SystemCallParameterInterfaceType(), oldfd(ofdp), newfd(nfdp), flags(flagsp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fildes;
            return true;
        }
        else if(parameter_index == 1) {
            param = fildes;
            return true;
        }
        else if(parameter_index == 2) {
            param = fildes;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Dup3 : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = Dup3InterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Dup3() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF