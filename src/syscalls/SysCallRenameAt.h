//
// SysCallRenameAt.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLRENAMEAT_H__
#define __SYSTEMCALLRENAMEAT_H__

#include "SystemCallInterface.h"
#include <type_traits>
#include <sys/types.h>
#include <string>

//     int
//     renameat(int fromfd, const char *from, int tofd, const char *to);


namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
using RenameAtSystemCallParametersInterfaceType = SystemCallInterface<RiscvArchType, 38>;

template<typename RiscvArchType=Riscv32>
class RenameAtSystemCallParameters : public virtual RenameAtSystemCallParametersInterfaceType<RiscvArchType> {
    
    private:

    const int fromfd;
    const std::string from;
    const int tofd;
    const std::string to;

    public:

    using SystemCallParameterInterfaceType = RenameAtSystemCallParametersInterfaceType<RiscvArchType>;
    using SystemCallCodeType = typename SystemCallParameterInterfaceType::SystemCallCodeType;

    RenameAtSystemCallParameters(int frfd, const std::string fromstr, int tfd, const std::string to_)
        : SystemCallParameterInterfaceType(), fromfd(frfd), from(fromstr), tofd(tfd), to(to_) {}

    size_t count() override { return 4UL; }

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);

    template<>
    bool get(const size_t parameter_index, int& param) {
        if(parameter_index == 0) {
            param = fromfd;
            return true;
        }
        else if(parameter_index == 2) {
            param = tofd;
            return true;
        }
        
        return false;
    }

    template<>
    bool get(const size_t parameter_index, std::string & param) {
        if(parameter_index == 1) {
            param = from;
            return true;
        }
        else if(parameter_index == 3) {
            param = to;
            return true;
        }        
        
        return false;
    }
};

template<typename RiscvArchType=Riscv32>
using RenameAtSystemCallInterfaceType = SystemCallInterface<RiscvArchType, 93>;

template<typename RiscvArchType=Riscv32>
class RenameAtSystemCall : public virtual RenameAtSystemCallInterfaceType<RiscvArchType> {
  
    public:

    using SystemCallInterfaceType = RenameAtSystemCallInterfaceType<RiscvArchType>;

    using RiscvModeIntegerType = typename SystemCallInterfaceType::RiscvModeIntegerType;
    using SystemCallCodeType = typename SystemCallInterfaceType::SystemCallCodeType;
    
    using SystemCallParameterInterfaceType = SystemCallParameterInterface<RiscvArchType, SystemCallInterfaceType::SystemCallCodeType::value>;    

    RenameAtSystemCall() : SystemCallInterfaceType() {}

    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterfaceType & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterfaceType & parameters, ssize_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF