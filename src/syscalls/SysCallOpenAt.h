//
// SysCallOpenAt.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLOPENAT_H__
#define __SYSTEMCALLOPENAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>

#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {


template<typename RiscvArchType=Riscv32>
using OpenAtSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 56>;

template<typename RiscvArchType=Riscv32>
class OpenAtSystemCallParameters : public virtual OpenAtSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    std::string path;
    int oflag;
    mode_t mode;

    public:

    using SystemCallParameterInterfaceType = OpenAtSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    OpenAtSystemCallParameters(int fd_i, std::string path_i, int oflag_i, mode_t mode_i)
        : SystemCallParameterInterfaceType(), fd(fd_i), path(path_i), oflag(oflag_i), mode(mode_i) {}

    size_t count() override { return 4UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string & param) {
        if(parameter_index == 1) {
            param = path;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 2) {
            param = oflag;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, mode_t & param) {
        if(parameter_index == 3) {
            param = mode;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using OpenAtSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 56>;

template<typename RiscvArchType=Riscv32>
class OpenAtSystemCall : public virtual SystemCallInterface<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = OpenAtSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType>;

    OpenAtSystemCall() : SystemCallInterfaceType() {}

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