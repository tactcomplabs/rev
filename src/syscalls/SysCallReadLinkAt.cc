//
// SysCallReadLinkAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallReadLinkAt.h"

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool ReadlinkatParameters<RiscvArchType>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = path;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool ReadlinkatParameters<RiscvArchType>::get<char*>(const size_t parameter_index, char* & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool ReadlinkatParameters<RiscvArchType>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 2) {
        param = bufsize;
        return true;
    }

    return false;
}

template<>
template<>
void Readlinkat<Riscv32>::invoke<ssize_t>(Readlinkat<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        std::string path;
        char * buf;
        size_t bufsize;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, path);
        has_values[1] = parameters.get<char *>(1, buf);
        has_values[2] = parameters.get<size_t>(2, bufsize);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = readlink(path, buf, bufsize);
        }
    }
}

template<>
template<>
void Readlinkat<Riscv64>::invoke<ssize_t>(Readlinkat<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        std::string path;
        char * buf;
        size_t bufsize;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, path);
        has_values[1] = parameters.get<char *>(1, buf);
        has_values[2] = parameters.get<size_t>(2, bufsize);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = readlink(path, buf, bufsize);
        }
    }
}

template<>
template<>
void Readlinkat<Riscv128>::invoke<ssize_t>(Readlinkat<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 3) {

        std::string path;
        char * buf;
        size_t bufsize;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, path);
        has_values[1] = parameters.get<char *>(1, buf);
        has_values[2] = parameters.get<size_t>(2, bufsize);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = readlink(path, buf, bufsize);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
