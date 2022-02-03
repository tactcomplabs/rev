//
// SysCallStat.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLLSTAT_H__
#define __SYSTEMCALLLSTAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <string>
#include <sys/types.h>
#include <string>

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using LstatSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 1039>;

template<typename RiscvArchType=Riscv32>
class LstatSystemCallParameters : public virtual LstatSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    std::string pth;
    stat * buf;

    public:

    using SystemCallParameterInterfaceType = LstatSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    LstatSystemCallParameters(std::string path, stat * bufi)
        : SystemCallParameterInterfaceType(), pth(path), buf(bufi) {}

    size_t count() override {
        return 4UL;
    }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, std::string& param) {
        if(parameter_index == 0) {
            param = pth;
            return true;
        }

        return false;
    }

    template<>
    bool get(const size_t parameter_index, stat * param) {
        if(parameter_index == 1) {
            param = buf;
            return true;
        }

        return false;
    }    
};

template<typename RiscvArchType=Riscv32>
using LstatSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 1039>;

template<typename RiscvArchType=Riscv32>
class LstatSystemCall : public virtual LstatSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = LstatSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    LstatSystemCall() : SystemCallInterfaceType() {}

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