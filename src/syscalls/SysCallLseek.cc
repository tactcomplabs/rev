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

template<>
template<>
void LseekSystemCall<Riscv32>::invoke<off_t>(LseekSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, off_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        off_t offset = -1;
        int whence = -1;

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

template<>
template<>
void LseekSystemCall<Riscv64>::invoke<off_t>(LseekSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, off_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        off_t offset = -1;
        int whence = -1;

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

template<>
template<>
void LseekSystemCall<Riscv128>::invoke<off_t>(LseekSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, off_t & value) {
    if(parameters.count() == 3) {
        int fd = -1;
        off_t offset = -1;
        int whence = -1;

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