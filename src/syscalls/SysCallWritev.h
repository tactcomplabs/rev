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
#include <sys/uio.h>
#include <unistd.h>

namespace SST { namespace RevCPU {

using iovec_t = struct iovec;

template<typename RiscvArchType=Riscv32>
using WritevInterfaceType = SystemCallInterfaceCode<RiscvArchType, 66>;

template<typename RiscvArchType=Riscv32>
class WritevParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    iovec_t * iov;
    int iovcnt;

    public:

    using SystemCallParameterInterfaceType = WritevParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    WritevParameters(int fildesp, iovec * iovp, int iovcntp)
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
    bool get(const size_t parameter_index, iovec_t * & param) {
        if(parameter_index == 0) {
            param = iov;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
class Writev : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = WritevInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Writev() : SystemCallInterfaceType() {}

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