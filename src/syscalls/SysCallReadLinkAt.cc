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

template<>
template<>
void ReadLinkAtSystemCall<Riscv32>::invoke<ssize_t>(ReadLinkAtSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
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
void ReadLinkAtSystemCall<Riscv64>::invoke<ssize_t>(ReadLinkAtSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
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
void ReadLinkAtSystemCall<Riscv128>::invoke<ssize_t>(ReadLinkAtSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
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