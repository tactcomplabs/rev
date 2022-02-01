//
// SysCallFstatat.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallFstatat.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void FstatatSystemCall<Riscv32>::invoke<int>(FstatatSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fd;
        std::string path;
        stat * buf;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, fd);
        has_values[1] = parameters.get<size_t>(1, path);
        has_values[2] = parameters.get<size_t>(2, buf);
        has_values[3] = parameters.get<size_t>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            fstatat(fd, const_cast<char *>(path.c_str()), buf, flag_);
        }
    }
}

template<>
template<>
void FstatatSystemCall<Riscv64>::invoke<int>(FstatatSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fd;
        std::string path;
        stat * buf;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, fd);
        has_values[1] = parameters.get<size_t>(1, path);
        has_values[2] = parameters.get<size_t>(2, buf);
        has_values[3] = parameters.get<size_t>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            fstatat(fd, const_cast<char *>(path.c_str()), buf, flag_);
        }
    }
}

template<>
template<>
void FstatatSystemCall<Riscv128>::invoke<int>(FstatatSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fd;
        std::string path;
        stat * buf;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, fd);
        has_values[1] = parameters.get<size_t>(1, path);
        has_values[2] = parameters.get<size_t>(2, buf);
        has_values[3] = parameters.get<size_t>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            fstatat(fd, const_cast<char *>(path.c_str()), buf, flag_);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST