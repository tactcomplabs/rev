//
// SysCallGetcwd.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallGetcwd.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool GetcwdParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool GetcwdParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool GetcwdParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool GetcwdParameters<Riscv32>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 0) {
        param = size;
        return true;
    }

    return false;
}

template<>
template<>
bool GetcwdParameters<Riscv64>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 0) {
        param = size;
        return true;
    }

    return false;
}

template<>
template<>
bool GetcwdParameters<Riscv128>::get<size_t>(const size_t parameter_index, size_t& param) {
    if(parameter_index == 0) {
        param = size;
        return true;
    }

    return false;
}

template<>
template<>
void Getcwd<Riscv32>::invoke<std::string>(RevRegFile const& memregfile, RevMem const& revmemory, std::string & value) {
    if(parameters.count() == 2) {

        std::string pth;
        size_t size;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, pth);
        has_values[1] = parameters.get<size_t>(1, size);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = std::string{getcwd(const_cast<char*>(pth.c_str()), size), size};
        }
    }
}

template<>
template<>
void Getcwd<Riscv64>::invoke<std::string>(RevRegFile const& memregfile, RevMem const& revmemory, std::string & value) {
    if(parameters.count() == 2) {

        std::string pth;
        size_t size;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, pth);
        has_values[1] = parameters.get<size_t>(1, size);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = std::string{getcwd(const_cast<char*>(pth.c_str()), size), size};
        }
    }
}

template<>
template<>
void Getcwd<Riscv128>::invoke<std::string>(RevRegFile const& memregfile, RevMem const& revmemory, std::string & value) {
    if(parameters.count() == 2) {

        std::string pth;
        size_t size;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, pth);
        has_values[1] = parameters.get<size_t>(1, size);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = std::string{getcwd(const_cast<char*>(pth.c_str()), size), size};
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
