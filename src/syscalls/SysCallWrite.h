//
// SysCallWrite.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLWRITE_H__
#define __SYSTEMCALLWRITE_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using WriteParametersInterfaceType = SystemCallInterface<RiscvArchType, 64>;

template<typename RiscvArchType=Riscv32>
class WriteParameters : public virtual WriteParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    void_ptr buf;
    size_t bcount;

    public:

    using SystemCallParameterInterfaceType = WriteParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    WriteParameters(int fd_i, void *buf_i, size_t count_i) : SystemCallParameterInterfaceType(), fd(fd_i), buf(buf_i), bcount(count_i) {}

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
        if(parameter_index == 1) {
            param = buf;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 2) {
            param = bcount;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using WriteInterfaceType = SystemCallInterfaceCode<RiscvArchType, 64>;

template<typename RiscvArchType=Riscv32>
class Write : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = WriteInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;    
    
    Write() : SystemCallInterfaceType() {}

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