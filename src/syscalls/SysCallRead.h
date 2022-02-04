//
// SysCallRead.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLREAD_H__
#define __SYSTEMCALLREAD_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using ReadSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 63>;

template<typename RiscvArchType=Riscv32>
class ReadSystemCallParameters : public virtual ReadSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    void_ptr buf;
    size_t bcount;

    public:

    using SystemCallParameterInterfaceType = ReadSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    ReadSystemCallParameters(int fd_i, void *buf_i, size_t count_i) : SystemCallParameterInterfaceType(), fd(fd_i), buf(buf_i), bcount(count_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, void_ptr & param) {
        if(parameter_index == 0) {
            param = buf;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 0) {
            param = bcount;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using ReadSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 63>;

template<typename RiscvArchType=Riscv32>
class ReadSystemCall : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = ReadSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    ReadSystemCall() : SystemCallInterfaceType() {}

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