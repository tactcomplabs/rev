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
void AccessSystemCall<Riscv32>::invoke<int>(AccessSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
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
void AccessSystemCall<Riscv64>::invoke<int>(AccessSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
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
void AccessSystemCall<Riscv128>::invoke<int>(AccessSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
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