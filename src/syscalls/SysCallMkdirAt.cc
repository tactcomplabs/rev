//
// SysCallMkdirAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMkdirAt.h"

#include <unistd.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool MkdiratParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}
    
template<>
template<>
bool MkdiratParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv32>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 2) {
        param = bcount;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv64>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 2) {
        param = bcount;
        return true;
    }

    return false;
}

template<>
template<>
bool MkdiratParameters<Riscv128>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 2) {
        param = bcount;
        return true;
    }

    return false;
}

template<>
template<>
void Mkdirat<Riscv32>::invoke<int>(SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        std::string buf;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[1]) {
            success = true;
            value = mkdirat(fd, buf.c_str(), count);
        }
    }
}

template<>
template<>
void Mkdirat<Riscv64>::invoke<int>(SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        std::string buf;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[1]) {
            success = true;
            value = mkdirat(fd, buf.c_str(), count);
        }
    }
}

template<>
template<>
void Mkdirat<Riscv128>::invoke<int>(SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        std::string buf;
        size_t count = 0;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<std::string>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[1]) {
            success = true;
            value = mkdirat(fd, buf.c_str(), count);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
