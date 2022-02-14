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
using PreadInterfaceType = SystemCallInterfaceCode<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class PreadParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fildes;
    void_ptr buf;
    size_t nbyte;
    off_t offset;

    public:

    PreadParameters(int fd_i, void *buf_i, size_t nbyte_i, off_t offset_i)
        : fildes(fd_i), buf(nbyte_i), nbyte(nbyte_i), offset(offset_i) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Pread : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = PreadInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Pread() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
