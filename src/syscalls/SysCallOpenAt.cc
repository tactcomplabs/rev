//
// SysCallOpenAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallOpenAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>

namespace SST { namespace RevCPU {

size_t OpenAtSystemCallParameters::count() {
    return count;
}

template<> inline
bool OpenAtSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

using char_ptr = char *;

template<> inline
bool OpenAtSystemCallParameters::get<char_ptr>(const size_t parameter_index, char_ptr & param) {
    if(parameter_index == 1) {
        strncpy(buf.c_str(), param, buf.size());
        return true;
    }

    return false;
}

template<> inline
bool OpenAtSystemCallParameters::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = std::string{path};
        return true;
    }

    return false;
}

template<> inline
bool OpenAtSystemCallParameters::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 2) {
        param = oflag;
        return true;
    }

    return false;
}

template<> inline
bool OpenAtSystemCallParameters::get<mode_t>(const size_t parameter_index, int& param) {
    if(parameter_index == 3) {
        param = mode;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
typename OpenAtSystemCall<RiscvArchType>::RiscvModeIntegerType ReadSystemCall<RiscvArchType>::code() {
    return OpenAtSystemCall<RiscvArchType>::code_value;
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
void OpenAtSystemCall<Riscv32>::invoke<ssize_t>(SystemCallParameterInterface & parameters, ssize_t & value) {
    invoke_impl(parameters, value, success);

}

} /* end namespace RevCPU */ } // end namespace SST