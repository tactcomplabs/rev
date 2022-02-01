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
#ifndef __SYSTEMCALLUNLINKAT_H__
#define __SYSTEMCALLUNLINKAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {


template<typename RiscvArchType=Riscv32>
using UnlinkAtSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class UnlinkAtSystemCallParameters : public virtual UnlinkAtSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    const int dirfd;
    const std::string pth;
    const int flags;

    public:

    using SystemCallParameterInterfaceType = UnlinkAtSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    UnlinkAtSystemCallParameters(const int dirfd_, cchar_ptr path, const size_t pathsz, const int flags_)
        : SystemCallParameterInterfaceType(), dirfd(dirfd_), pth(path), flags(flags_)  {}

    size_t count() override { return 3UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = dirfd;
            return true;
        }
        else if(parameter_index == 2) {
            param = flags;
            return true;
        }
        return false;
    }    

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 1) {
            param = pth;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using UnlinkAtSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class UnlinkAtSystemCall : public virtual UnlinkAtSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = UnlinkAtSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    UnlinkAtSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF