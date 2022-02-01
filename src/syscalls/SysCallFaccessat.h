//
// SysCallFstat.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFSTAT_H__
#define __SYSTEMCALLFSTAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using FaccessatSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class FaccessatSystemCallParameters : public virtual FaccessatSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    std::string pth;
    int mode;
    int flag;

    public:

    using SystemCallParameterInterfaceType = FaccessatSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    FaccessatSystemCallParameters(int fdi, std::string path, int modei, int flagi)
        : SystemCallParameterInterfaceType(), fd(fdi), pth(path), mode(modei), flag(flagi) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }
        else if(parameter_index == 2) {
            param = mode;
            return true;
        }
        else if(parameter_index == 3) {
            param = flag;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string param) {
        if(parameter_index == 1) {
            param = pth;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using FaccessatSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class FaccessatSystemCall : public virtual FaccessatSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FaccessatSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    FaccessatSystemCall() : SystemCallInterfaceType() {}

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