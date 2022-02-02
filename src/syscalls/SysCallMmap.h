//
// SysCallMmap.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLMMAP_H__
#define __SYSTEMCALLMMAP_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using MmapSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class MmapSystemCallParameters : public virtual MmapSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    void * addr;
    size_t len;
    int prot;
    int flags;
    int fd;
    off_t offset;

    public:

    using SystemCallParameterInterfaceType = MmapSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    MmapSystemCallParameters(void * addrp, size_t lenp, int protp, int flagsp, int fdp, off_t offsetp)
        : SystemCallParameterInterfaceType(), addr(addrp), len(lenp), prot(protp), flags(flagsp), fd(fdp), offset(offsetp) {}

    size_t count() override { return 6UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, void_ptr & param) {
        if(parameter_index == 0) {
            param = addr;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, size_t & param) {
        if(parameter_index == 1) {
            param = len;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 2) {
            param = prot;
            return true;
        }
        else if(parameter_index == 3) {
            param = flags;
            return true;
        }
        else if(parameter_index == 4) {
            param = fd;
            return true;
        }
        else if(parameter_index == 5) {
            param = offset;
            return true;
        }

        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using MmapSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class MmapSystemCall : public virtual MmapSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = MmapSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    public:

    MmapSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, void_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF