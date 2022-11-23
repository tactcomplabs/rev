//
// SysCallOpen.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallOpen.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool OpenParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 0) {
        param = path;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 0) {
        param = path;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 0) {
        param = path;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv32>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 1) {
        param = oflag;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv64>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 1) {
        param = oflag;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv128>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 1) {
        param = oflag;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv32>::get<mode_t>(const size_t parameter_index, mode_t & param) {
    if(parameter_index == 2) {
        param = mode;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv64>::get<mode_t>(const size_t parameter_index, mode_t & param) {
    if(parameter_index == 2) {
        param = mode;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenParameters<Riscv128>::get<mode_t>(const size_t parameter_index, mode_t & param) {
    if(parameter_index == 2) {
        param = mode;
        return true;
    }

    return false;
}

template<>
template<>
void Open<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 3) {
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, path);
        has_values[1] = parameters.get<int>(1, oflag);
        has_values[2] = parameters.get<mode_t>(2, mode);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = open(path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void Open<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 3) {
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, path);
        has_values[1] = parameters.get<int>(1, oflag);
        has_values[2] = parameters.get<mode_t>(2, mode);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = open(path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void Open<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 3) {
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[5] = { false, false, false, false, false };
        has_values[0] = parameters.get<std::string>(0, path);
        has_values[1] = parameters.get<int>(1, oflag);
        has_values[2] = parameters.get<mode_t>(2, mode);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = open(path.c_str(), oflag, mode);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST