//
// SysCallRead.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallRead.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

size_t ReadSystemCallParameters::count() {
    return 3UL;
}

template<> inline
bool ReadSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

using void_ptr = void *;

template<> inline
bool ReadSystemCallParameters::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 0) {
        param = buf;
        return true;
    }

    return false;
}

template<> inline
bool ReadSystemCallParameters::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 0) {
        param = bcount;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename ReadSystemCall<RiscvArchType>::RiscvModeIntegerType ReadSystemCall<RiscvArchType>::code() {
    return ReadSystemCall<RiscvArchType>::code_value;
}

static void invoke_impl(SystemCallParameterInterface & parameters, ssize_t & value, bool & invoc_success) {
    if(parameters.count() == 3) {
        int fd = -1;
        void * buf = 0;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void*>(0, buf);
        has_values[2] = parameters.get<size_t>(0, count);

        if(has_values[0] && has_values[1] && has_values[1] && fd != -1 && buf != 0 && count != -1) {
            invoc_success = true;
            value = read(fd, buf, count);
        }
    }

    invoc_success = false;
}

template<>
template<>
void ReadSystemCall<Riscv32>::invoke<ssize_t>(SystemCallParameterInterface & parameters, ssize_t & value) {
    invoke_impl(parameters, value, success);

}

template<>
template<>
void ReadSystemCall<Riscv64>::invoke<ssize_t>(SystemCallParameterInterface & parameters, ssize_t & value) {
    invoke_impl(parameters, value, success);
}

template<>
template<>
void ReadSystemCall<Riscv128>::invoke<ssize_t>(SystemCallParameterInterface & parameters, ssize_t & value) {
    invoke_impl(parameters, value, success);
}

} /* end namespace RevCPU */ } // end namespace SST