//
// SysCallRenameAt.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallRenameAt.h"

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
template<>
bool RenameatParameters<RiscvArchType>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fromfd;
        return true;
    }
    else if(parameter_index == 2) {
        param = tofd;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool RenameatParameters<RiscvArchType>::get<std::string>(const size_t parameter_index, std::string& param) {
    if(parameter_index == 1) {
        param = from;
        return true;
    }
    else if(parameter_index == 2) {
        param = to;
        return true;
    }

    return false;
}

template<>
template<>
void Renameat<Riscv32>::invoke<ssize_t>(Renameat<Riscv32>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {

        int fromfd;
        std::string from;
        int tofd;
        std::string to;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fromfd);
        has_values[1] = parameters.get<std::string>(1, from);
        has_values[2] = parameters.get<int>(2, tofd);
        has_values[3] = parameters.get<std::string>(3, to);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = renameat(fromfd, from.c_str(), tofd, to.c_str());
        }
    }
}

template<>
template<>
void Renameat<Riscv64>::invoke<ssize_t>(Renameat<Riscv64>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {

        int fromfd;
        std::string from;
        int tofd;
        std::string to;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fromfd);
        has_values[1] = parameters.get<std::string>(1, from);
        has_values[2] = parameters.get<int>(2, tofd);
        has_values[3] = parameters.get<std::string>(3, to);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = renameat(fromfd, from.c_str(), tofd, to.c_str());
        }
    }
}

template<>
template<>
void Renameat<Riscv128>::invoke<ssize_t>(Renameat<Riscv128>::SystemCallParameterInterfaceType & parameters, ssize_t & value) {
    if(parameters.count() == 4) {

        int fromfd;
        std::string from;
        int tofd;
        std::string to;

        bool has_values[4] = { false, false, false, false };
        has_values[0] = parameters.get<int>(0, fromfd);
        has_values[1] = parameters.get<std::string>(1, from);
        has_values[2] = parameters.get<int>(2, tofd);
        has_values[3] = parameters.get<std::string>(3, to);

        if(has_values[0] && has_values[1] && has_values[2] && has_values[3]) {
            success = true;
            value = renameat(fromfd, from.c_str(), tofd, to.c_str());
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
