//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallLinkAt.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool LinkatParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = oldpth;
        return true;
    }
    else if(parameter_index == 3) {
        param = newpth;
        return true;
    }

    return false;
}

template<>
template<>
bool LinkatParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd1;
        return true;
    }
    else if(parameter_index == 2) {
        param = fd2;
        return true;
    }
    else if(parameter_index == 4) {
        param = flag;
        return true;
    }

    return false;
}

template<>
template<>
void Linkat<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 5) {
        std::string oldpth{};
        std::string newpth{};
        int fd1, fd2, flag;
        
        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd1);
        has_values[1] = parameters.get<std::string>(1, oldpth);
        has_values[2] = parameters.get<int>(2, fd2);
        has_values[3] = parameters.get<std::string>(3, newpth);
        has_values[4] = parameters.get<int>(4, flag);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = linkat(fd1, oldpth.c_str(), fd2, oldpth.c_str(), flag);
        }
    }
}

template<>
template<>
void Linkat<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 5) {
        std::string oldpth{};
        std::string newpth{};
        int fd1, fd2, flag;
        
        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd1);
        has_values[1] = parameters.get<std::string>(1, oldpth);
        has_values[2] = parameters.get<int>(2, fd2);
        has_values[3] = parameters.get<std::string>(3, newpth);
        has_values[4] = parameters.get<int>(4, flag);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = linkat(fd1, oldpth.c_str(), fd2, oldpth.c_str(), flag);
        }
    }
}

template<>
template<>
void Linkat<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 5) {
        std::string oldpth{};
        std::string newpth{};
        int fd1, fd2, flag;
        
        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd1);
        has_values[1] = parameters.get<std::string>(1, oldpth);
        has_values[2] = parameters.get<int>(2, fd2);
        has_values[3] = parameters.get<std::string>(3, newpth);
        has_values[4] = parameters.get<int>(4, flag);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = linkat(fd1, oldpth.c_str(), fd2, oldpth.c_str(), flag);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
