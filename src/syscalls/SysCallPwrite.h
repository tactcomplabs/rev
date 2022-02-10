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
using PwriteInterfaceType = SystemCallInterfaceCode<RiscvArchType, 67>;

template<typename RiscvArchType=Riscv32>
class PwriteParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    void_ptr buf;
    size_t nbyte;
    off_t offset;

    public:

    PwriteParameters(int fd_i, void *buf_i, size_t nbyte_i, off_t offset_i)
        : fildes(fd_i), buf(buf_i), nbyte(nbyte_i), offset(offset_i) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Pwrite : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = PwriteInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Pwrite() {}

    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
