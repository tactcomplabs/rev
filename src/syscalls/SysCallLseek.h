//
// SysCallExit.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLLSEEK_H__
#define __SYSTEMCALLLSEEK_H__

#include "SystemCallInterface.h"
#include <type_traits>

namespace SST { namespace RevCPU {

class LseekSystemCallParameters : public virtual SystemCallParameterInterface {
    
    int fd;
    off_t offset;
    int whence;

    public:

    LseekSystemCallParameters(const int fd_, const off_t offset_, const int whence_) : SystemCallParameterInterface(), fd(fd_), offset(offset_), whence(whence_) {}

    size_t count() override;

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class LseekSystemCall : public virtual SystemCallInterface<RiscvArchType> {

    using RiscvModeIntegerType = typename SystemCallInterface<RiscvArchType>::RiscvModeIntegerType;
    
    public:

    const static RiscvModeIntegerType code_value = static_cast<RiscvModeIntegerType>(62);

    LseekSystemCall() {}

    RiscvModeIntegerType code() override;
    
    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterface & parameters, ReturnType & value);

    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterface & parameters, off_t & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF