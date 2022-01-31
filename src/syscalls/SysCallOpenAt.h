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

class OpenAtSystemCallParameters : public virtual SystemCallParameterInterface {

    private:

    size_t count;
    int fd;
    std::string path;
    int oflag;
    mode_t mode;

    public:

    OpenAtSystemCallParameters(int fd_i, size_t path_i_len, char *path_i, size_t oflag_i, mode_t mode_i=-1) : SystemCallParameterInterface(), fd(fd_i), oflag(oflag_i), mode(mode_i) {
        path = std::string{buf_i, buf_i_len};
        count = (mode_i == -1) ? 3UL : 4UL;
    }

    OpenAtSystemCallParameters(int fd_i, std::string & path_i, size_t oflag_i, mode_t mode_i=-1) : SystemCallParameterInterface(), fd(fd_i), oflag(oflag_i), mode(mode_i) {
        path = std::string{path_i.c_str(), path_i.size()};
        count = (mode_i == -1) ? 3UL : 4UL;
    }

    size_t count() override;

    template<typename ParameterType>
    bool get(const size_t parameter_index, ParameterType & param);
};

template<typename RiscvArchType=Riscv32>
class OpenAtSystemCall : public virtual SystemCallInterface<RiscvArchType> {

    using RiscvModeIntegerType = typename SystemCallInterface<RiscvArchType>::RiscvModeIntegerType;

    public:

    const static RiscvModeIntegerType code_value = static_cast<RiscvModeIntegerType>(56);

    OpenAtSystemCall() {}

    RiscvModeIntegerType code() override;
    
    // always returns false
    //
    template<typename ReturnType>
    void invoke(SystemCallParameterInterface & parameters, ReturnType & value);
    
    // returns true
    //
    template<>
    void invoke(SystemCallParameterInterface & parameters, int & value);
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF