//
// SysCallRenameAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallRenameAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
void RenameAtSystemCall<Riscv32>::invoke<ssize_t>(RenameAtSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {

        int fromfd;
        std::string from;
        int tofd;
        std::string to;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fromfd);
        has_values[1] = parameters.get<std::string>(1, from);
        has_values[2] = parameters.get<int>(2, tofd);
        has_values[3] = parameters.get<std::string>(3, to);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = renameat(fromfd, from.c_str(), tofd, to.c_str());
        }
    }
}

template<>
template<>
void RenameAtSystemCall<Riscv64>::invoke<ssize_t>(RenameAtSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {

        int fromfd;
        std::string from;
        int tofd;
        std::string to;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fromfd);
        has_values[1] = parameters.get<std::string>(1, from);
        has_values[2] = parameters.get<int>(2, tofd);
        has_values[3] = parameters.get<std::string>(3, to);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = renameat(fromfd, from.c_str(), tofd, to.c_str());
        }
    }
}

template<>
template<>
void RenameAtSystemCall<Riscv128>::invoke<ssize_t>(RenameAtSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {

        int fromfd;
        std::string from;
        int tofd;
        std::string to;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fromfd);
        has_values[1] = parameters.get<std::string>(1, from);
        has_values[2] = parameters.get<int>(2, tofd);
        has_values[3] = parameters.get<std::string>(3, to);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = renameat(fromfd, from.c_str(), tofd, to.c_str());
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST