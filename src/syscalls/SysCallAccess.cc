//
// SysCallAccess.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallAccess.h"

#include <unistd.h>

namespace SST { namespace RevCPU {

    template<>
    template<>
    bool AccessParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
        if(parameter_index == 1) {
            param = mode;
            return true;
        }

        return false;
    }


    template<>
    template<>
    bool AccessParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
        if(parameter_index == 1) {
            param = mode;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool AccessParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
        if(parameter_index == 1) {
            param = mode;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool AccessParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string & param) {
        if(parameter_index == 0) {
            param = pth;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool AccessParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string & param) {
        if(parameter_index == 0) {
            param = pth;
            return true;
        }

        return false;
    }

    template<>
    template<>
    bool AccessParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string & param) {
        if(parameter_index == 0) {
            param = pth;
            return true;
        }

        return false;
    }

template<>
template<>
void Access<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        std::string pth;
        int mode;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, pth);
        has_values[1] = parameters.get<int>(1, mode);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = access(const_cast<char *>(pth.c_str()), mode);
        }
    }
}

template<>
template<>
void Access<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {

        std::string pth;
        int mode;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, pth);
        has_values[1] = parameters.get<int>(1, mode);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = access(const_cast<char *>(pth.c_str()), mode);
        }
    }
}

template<>
template<>
void Access<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    //Access<Riscv128>::SystemCallParameterInterfaceType & parameters();
    if(parameters.count() == 2) {

        std::string pth;
        int mode;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, pth);
        has_values[1] = parameters.get<int>(1, mode);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = access(const_cast<char *>(pth.c_str()), mode);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
