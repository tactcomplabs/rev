//
// SysCallStatx.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallStatx.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Statx<Riscv32>::invoke<int>(Statx<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 5) {

        int dirfd;
        const char * pathname;
        int flags;
        unsigned int mask;
        statx * statxbuf;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, ofd);
        has_values[1] = parameters.get<const char *>(1, nfd);
        has_values[2] = parameters.get<int>(2, flags);
        has_values[3] = parameters.get<unsigned int>(3, flags);
        has_values[4] = parameters.get<statx *>(4, flags);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]  && has_values[4]) {
            success = true;
            value = statx(dirfd, pathname, flags, mask, statxbuf);
        }
    }
}

template<>
template<>
void Statx<Riscv64>::invoke<int>(Statx<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 5) {

        int dirfd;
        const char * pathname;
        int flags;
        unsigned int mask;
        statx * statxbuf;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, ofd);
        has_values[1] = parameters.get<const char *>(1, nfd);
        has_values[2] = parameters.get<int>(2, flags);
        has_values[3] = parameters.get<unsigned int>(3, flags);
        has_values[4] = parameters.get<statx *>(4, flags);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]  && has_values[4]) {
            success = true;
            value = statx(dirfd, pathname, flags, mask, statxbuf);
        }
    }
}

template<>
template<>
void Statx<Riscv128>::invoke<int>(Statx<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 5) {

        int dirfd;
        const char * pathname;
        int flags;
        unsigned int mask;
        statx * statxbuf;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, ofd);
        has_values[1] = parameters.get<const char *>(1, nfd);
        has_values[2] = parameters.get<int>(2, flags);
        has_values[3] = parameters.get<unsigned int>(3, flags);
        has_values[4] = parameters.get<statx *>(4, flags);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]  && has_values[4]) {
            success = true;
            value = statx(dirfd, pathname, flags, mask, statxbuf);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST