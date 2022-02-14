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
using WriteInterfaceType = SystemCallInterfaceCode<RiscvArchType, 64>;

template<typename RiscvArchType=Riscv32>
class WriteParameters : public virtual SystemCallParameterInterface<RiscvArchType> {
    
    private:

    int fd;
    void_ptr buf;
    size_t bcount;

    public:

    WriteParameters(int fd_i, void *buf_i, size_t count_i)
        : fd(fd_i), buf(buf_i), bcount(count_i) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Write : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = WriteInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;    
    
    Write() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
