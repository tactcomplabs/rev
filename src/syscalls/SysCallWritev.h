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

    WritevParameters(int fildesp, iovec * iovp, int iovcntp)
        : fildes(fildesp), iov(iovp), iovcnt(iovcntp) {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class Writev : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = WritevInterfaceType<RiscvArchType>;

    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    Writev() {}

    template<typename ReturnType>
    void invoke(RevRegFile const& memregfile, RevMem const& revmemory, ReturnType & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
