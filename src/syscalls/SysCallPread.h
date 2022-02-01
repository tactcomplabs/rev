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
#ifndef __SYSTEMCALLPREAD_H__
#define __SYSTEMCALLPREAD_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using PreadSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class PreadSystemCallParameters : public virtual PreadSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fildes;
    void_ptr buf;
    size_t nbyte;
    off_t offset;

    public:

    using SystemCallParameterInterfaceType = PreadSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    PreadSystemCallParameters(int fd_i, void *buf_i, size_t nbyte_i, off_t offset_i)
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
using PreadSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class PreadSystemCall : public virtual PreadSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = PreadSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    PreadSystemCall() : SystemCallInterfaceType() {}

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