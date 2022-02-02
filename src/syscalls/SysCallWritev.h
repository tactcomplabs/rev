//
// SysCallWritev.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLWRITEV_H__
#define __SYSTEMCALLWRITEV_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using WritevSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class WritevSystemCallParameters : public virtual WritevSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fildes;
    iovec * iov;
    int iovcnt;

    public:

    using SystemCallParameterInterfaceType = WritevSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    WritevSystemCallParameters(int fildesp, iovec * iovp, int iovcntp)
        : SystemCallParameterInterfaceType(), fildes(fildesp), iov(iovp), iovcnt(iovcntp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fildes;
            return true;
        }
        else if(parameter_index == 2) {
            param = iovcnt;
            return true;
        }        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, iovec * & param) {
        if(parameter_index == 0) {
            param = iov;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using WritevSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class WritevSystemCall : public virtual WritevSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = WritevSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    WritevSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, ssize_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF