//
// SysCallWrite.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallLink.h"
#include <unistd.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool LinkParameters<Riscv32>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = oldpth;
        return true;
    }
    else if(parameter_index == 1) {
        param = newpth;
        return true;
    }

    return false;
}

template<>
template<>
bool LinkParameters<Riscv64>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = oldpth;
        return true;
    }
    else if(parameter_index == 1) {
        param = newpth;
        return true;
    }

    return false;
}

template<>
template<>
bool LinkParameters<Riscv128>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 0) {
        param = oldpth;
        return true;
    }
    else if(parameter_index == 1) {
        param = newpth;
        return true;
    }

    return false;
}

template<>
template<>
void Link<Riscv32>::invoke<int>(Link<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {

     if(parameters.count() == 2) {
        std::string oldpth{};
        std::string newpth{};
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, oldpth);
        has_values[1] = parameters.get<std::string>(1, newpth);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = link(oldpth.c_str(), oldpth.c_str());
        }
    }
}

template<>
template<>
void Link<Riscv64>::invoke<int>(Link<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {

     if(parameters.count() == 2) {
        std::string oldpth{};
        std::string newpth{};
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, oldpth);
        has_values[1] = parameters.get<std::string>(1, newpth);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = link(oldpth.c_str(), oldpth.c_str());
        }
    }
}

template<>
template<>
void Link<Riscv128>::invoke<int>(Link<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {

     if(parameters.count() == 2) {
        std::string oldpth{};
        std::string newpth{};
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<std::string>(0, oldpth);
        has_values[1] = parameters.get<std::string>(1, newpth);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = link(oldpth.c_str(), oldpth.c_str());
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
