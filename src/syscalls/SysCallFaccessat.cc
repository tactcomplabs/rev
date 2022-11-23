//
// SysCallFstatat.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallFaccessat.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool FaccessatParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = mode;
        return true;
    }
    else if(parameter_index == 3) {
        param = flag;
        return true;
    }

    return false;
}

template<>
template<>
bool FaccessatParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = mode;
        return true;
    }
    else if(parameter_index == 3) {
        param = flag;
        return true;
    }

    return false;
}

template<>
template<>
bool FaccessatParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = mode;
        return true;
    }
    else if(parameter_index == 3) {
        param = flag;
        return true;
    }

    return false;
}

template<>
template<>
bool FaccessatParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool FaccessatParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool FaccessatParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
void Faccessat<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 4) {

        int fd;
        std::string pth;
        int mode;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, pth);
        has_values[2] = parameters.get<int>(2, mode);
        has_values[3] = parameters.get<int>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = faccessat(fd, const_cast<char *>(pth.c_str()), mode, flag);
        }
    }
}

template<>
template<>
void Faccessat<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 4) {

        int fd;
        std::string pth;
        int mode;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, pth);
        has_values[2] = parameters.get<int>(2, mode);
        has_values[3] = parameters.get<int>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = faccessat(fd, const_cast<char *>(pth.c_str()), mode, flag);
        }
    }
}

template<>
template<>
void Faccessat<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 4) {

        int fd;
        std::string pth;
        int mode;
        int flag;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, pth);
        has_values[2] = parameters.get<int>(2, mode);
        has_values[3] = parameters.get<int>(3, flag);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = faccessat(fd, const_cast<char *>(pth.c_str()), mode, flag);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST