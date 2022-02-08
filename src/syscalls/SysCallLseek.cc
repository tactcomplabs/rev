//
// SysCallExit.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallLseek.h"

namespace SST { namespace RevCPU {

template<typename RiscvArchType> 
template<>
bool Lseek<RiscvArchType>::get(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fd;
        return true;
    }
    else if(parameter_index == 2) {
        param = whence;
        return true;
    }

    return false;
}
       
template<typename RiscvArchType> 
template<>
bool Lseek<RiscvArchType>::get(const size_t parameter_index, off_t& param) {
    if(parameter_index == 1) {
        param = offset;
        return true;
    }

    return false;
}


template<typename RiscvArchType>
template<>
void Lseek<RiscvArchType>::invoke<off_t>(Lseek<RiscvArchType>::SystemCallParameterInterfaceType & parameters, off_t & value) {
    if(parameters.count() == 3) {
        int fd;
        off_t offset;
        int whence;

        bool has_value [3] = { false, false, false };
        has_value[0] = parameters.get<int>(0, fd);
        has_value[1] = parameters.get<off_t>(1, offset);
        has_value[2] = parameters.get<int>(2, whence);

        if(has_value[0] && has_value[1] && has_value[2]) {
            success = true;
            value = lseek(fd, offset, whence);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
