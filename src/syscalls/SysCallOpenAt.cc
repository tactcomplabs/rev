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

#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <string>

namespace SST { namespace RevCPU {

template<>
template<>
bool OpenatParameters<Riscv32>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv64>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv128>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = fd;
        return true;
    }

    return false;
}
    
template<>
template<>
bool OpenatParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = path;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = path;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string & param) {
    if(parameter_index == 1) {
        param = path;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv32>::get<mode_t>(const size_t parameter_index, mode_t & param) {
    if(parameter_index == 3) {
        param = mode;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv64>::get<mode_t>(const size_t parameter_index, mode_t & param) {
    if(parameter_index == 3) {
        param = mode;
        return true;
    }

    return false;
}

template<>
template<>
bool OpenatParameters<Riscv128>::get<mode_t>(const size_t parameter_index, mode_t & param) {
    if(parameter_index == 3) {
        param = mode;
        return true;
    }

    return false;
}

template<>
template<>
void Openat<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 4) {
        int fd;
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, path);
        has_values[2] = parameters.get<int>(2, oflag);
        has_values[3] = parameters.get<mode_t>(3, mode);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = openat(fd, path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void Openat<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 4) {
        int fd;
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, path);
        has_values[2] = parameters.get<int>(2, oflag);
        has_values[3] = parameters.get<mode_t>(3, mode);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = openat(fd, path.c_str(), oflag, mode);
        }
    }
}

template<>
template<>
void Openat<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 4) {
        int fd;
        std::string path;
        int oflag;
        mode_t mode;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, path);
        has_values[2] = parameters.get<int>(2, oflag);
        has_values[3] = parameters.get<mode_t>(3, mode);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = openat(fd, path.c_str(), oflag, mode);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST