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
#ifndef __SYSTEMCALLPWRITE_H__
#define __SYSTEMCALLPWRITE_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using PwriteSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 67>;

template<typename RiscvArchType=Riscv32>
class PwriteSystemCallParameters : public virtual WriteSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fildes;
    void_ptr buf;
    size_t nbyte;
    off_t offset;

    public:

    using SystemCallParameterInterfaceType = PwriteSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    PwriteSystemCallParameters(int fd_i, void *buf_i, size_t nbyte_i, off_t offset_i)
        : SystemCallParameterInterfaceType(), fd(fd_i), nbyte(nbyte_i), offset(offset_i) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fildes;
            return true;
        }
        else if(parameter_index == 3) {
            param = offset;
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
            param = nbyte;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using PwriteSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 67>;

template<typename RiscvArchType=Riscv32>
class PwriteSystemCall : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = PwriteSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    PwriteSystemCall() : SystemCallInterfaceType() {}

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