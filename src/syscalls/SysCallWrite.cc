//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallWrite.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool WriteParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv32>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv64>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv128>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv32>::get<size_t>(const size_t parameter_index, size_t & param) {
    if(parameter_index == 2) {
        param = bcount;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv64>::get<size_t>(const size_t parameter_index, size_t & param) {
    if(parameter_index == 2) {
        param = bcount;
        return true;
    }

    return false;
}

template<>
template<>
bool WriteParameters<Riscv128>::get<size_t>(const size_t parameter_index, size_t & param) {
    if(parameter_index == 2) {
        param = bcount;
        return true;
    }

    return false;
}

template<>
template<>
void Write<Riscv32>::invoke<ssize_t>(RevRegFile const& memregfile, RevMem const& revmemory, ssize_t & value) {
    if(parameters.count() == 3) {
        int fd;
        void * buf;
        size_t count;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = write(fd, buf, count);
        }
    }
}

template<>
template<>
void Write<Riscv64>::invoke<ssize_t>(RevRegFile const& memregfile, RevMem const& revmemory, ssize_t & value) {
    if(parameters.count() == 3) {
        int fd;
        void * buf;
        size_t count;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = write(fd, buf, count);
        }
    }
}

template<>
template<>
void Write<Riscv128>::invoke<ssize_t>(RevRegFile const& memregfile, RevMem const& revmemory, ssize_t & value) {
    if(parameters.count() == 3) {
        int fd;
        void * buf;
        size_t count;

        bool has_values[3] = { false, false, false };
        has_values[0] = parameters.get<int>(0, fd);
        has_values[1] = parameters.get<void_ptr>(1, buf);
        has_values[2] = parameters.get<size_t>(2, count);

        if(has_values[0] && has_values[1] && has_values[2]) {
            success = true;
            value = write(fd, buf, count);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST