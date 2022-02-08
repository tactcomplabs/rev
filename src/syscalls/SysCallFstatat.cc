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

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool FstatatParameters<RiscvArchType>::get(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 3) {
        param = flag;
        return true;
    }
    return false;
}

template<typename RiscvArchType>
template<>
bool FstatatParameters<RiscvArchType>::get(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = path;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool FstatatParameters<RiscvArchType>::get(const size_t parameter_index, stat_t * & param) {
    if(parameter_index == 2) {
        param = buf;
        return true;
    }

    return false;
}


template<>
template<>
void Fstatat<Riscv32>::invoke<int>(Fstatat<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fd;
        std::string path;
        stat_t * buf;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, path);
        has_values[2] = parameters.get<stat_t*>(2, buf);
        has_values[3] = parameters.get<int>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            fstatat(fd, const_cast<char *>(path.c_str()), buf, flag);
        }
    }
}

template<>
template<>
void Fstatat<Riscv64>::invoke<int>(Fstatat<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fd;
        std::string path;
        stat_t * buf;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, path);
        has_values[2] = parameters.get<stat_t*>(2, buf);
        has_values[3] = parameters.get<int>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            fstatat(fd, const_cast<char *>(path.c_str()), buf, flag);
        }
    }
}

template<>
template<>
void Fstatat<Riscv128>::invoke<int>(Fstatat<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fd;
        std::string path;
        stat_t * buf;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, path);
        has_values[2] = parameters.get<stat_t*>(2, buf);
        has_values[3] = parameters.get<int>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            fstatat(fd, const_cast<char *>(path.c_str()), buf, flag);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
