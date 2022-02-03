//
// SysCallOpen.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLOPEN_H__
#define __SYSTEMCALLOPEN_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>

#include <sys/types.h>
#include <fcntl.h>

namespace SST { namespace RevCPU {


template<typename RiscvArchType=Riscv32>
using OpenSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class OpenSystemCallParameters : public virtual OpenSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    std::string path;
    int oflag;
    mode_t mode;

    public:

    using SystemCallParameterInterfaceType = OpenSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    OpenSystemCallParameters(std::string path_i, int oflag_i, mode_t mode_i)
        : SystemCallParameterInterfaceType(), path(path_i), oflag(oflag_i), mode(mode_i) {}

    size_t count() override { return 4UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, std::string & param) {
        if(parameter_index == 0) {
            param = path;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, int & param) {
        if(parameter_index == 1) {
            param = oflag;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, mode_t & param) {
        if(parameter_index == 2) {
            param = mode;
            return true;
        }
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using OpenSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class OpenSystemCall : public virtual OpenSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = OpenSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    OpenSystemCall() : SystemCallInterfaceType() {}

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