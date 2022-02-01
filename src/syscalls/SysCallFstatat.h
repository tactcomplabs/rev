//
// SysCallFstatat.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLFSTATAT_H__
#define __SYSTEMCALLFSTATAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using FstatatSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class FstatatSystemCallParameters : public virtual FstatatSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    int fd;
    std::string path;
    stat * buf;
    int flag;

    public:

    using SystemCallParameterInterfaceType = FstatatSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    FstatatSystemCallParameters(int fdi, std::string pathi, stat * bufi, int flagi)
        : SystemCallParameterInterfaceType(), fd(fdi), path(pathi), buf(bufi), flag(flagi) {}

    size_t count() override {
        return (size == -1) ? 1UL : 2UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fd;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 0) {
            param = path;
            return true;
        }

        return false;
    }    

    template<>
    bool get(const size_t parameter_index, stat * & param) {
        if(parameter_index == 0) {
            param = buf;
            return true;
        }

        return false;
    }    

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = flag;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using FstatatSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 17>;

template<typename RiscvArchType=Riscv32>
class FstatatSystemCall : public virtual FstatatSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = FstatatSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    FstatatSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, std::string & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF