//
// SysCallUnlink.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallUnlink.h"
#include <algorithm>

#include <unistd.h>
#include <signal.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool UnlinkParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}
template<>
template<>
bool UnlinkParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
bool UnlinkParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = pth;
        return true;
    }

    return false;
}

template<>
template<>
void Unlink<Riscv32>::invoke<int>(Unlink<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        std::string pth{};
        
        const bool has_values = parameters.get<std::string>(0, pth);
        if(has_values) {
            success = true;
            value = unlink(pth.c_str());
        }
    }
}

template<>
template<>
void Unlink<Riscv64>::invoke<int>(Unlink<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        std::string pth{};

        const bool has_values = parameters.get<std::string>(0, pth);
        if(has_values) {
            success = true;
            value = unlink(pth.c_str());
        }
    }
}

template<>
template<>
void Unlink<Riscv128>::invoke<int>(Unlink<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 1) {
        std::string pth{};

        const bool has_values = parameters.get<std::string>(0, pth);
        if(has_values) {
            success = true;
            value = unlink(pth.c_str());
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
