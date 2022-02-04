//
// SysCallStatx.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLSTATX_H__
#define __SYSTEMCALLSTATX_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using StatxInterfaceType = SystemCallInterface<RiscvArchType, 291>;

template<typename RiscvArchType=Riscv32>
class StatxParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int dirfd;
    const char * pathname;
    int flags;
    unsigned int mask;
    statx * statxbuf;

    public:

    using SystemCallParameterInterfaceType = StatxParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    StatxParameters(int dirfdp, const char * pathnamep, int flagsp, unsigned int maskp, statx * statxbufp)
        : SystemCallParameterInterfaceType(), dirfd(dirfdp), flags(flagsp), mask(maskp), statxbuf(statxbuf) {}

    size_t count() override { return 5UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = dirfd;
            return true;
        }
        else if(parameter_index == 2) {
            param = flags;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, const char* & param) {
        if(parameter_index == 1) {
            param = pathname;
            return true;
        }

        return false;
    }    

    template<>
    bool get(const size_t parameter_index, unsigned int & param) {
        if(parameter_index == 3) {
            param = mask;
            return true;
        }
        
        return false;
    }        

    template<>
    bool get(const size_t parameter_index, statx * & param) {
        if(parameter_index == 4) {
            param = statxbuf;
            return true;
        }
        
        return false;
    }            
};

template<typename RiscvArchType=Riscv32>
class Statx : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = StatxInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Statx() : SystemCallInterfaceType() {}

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