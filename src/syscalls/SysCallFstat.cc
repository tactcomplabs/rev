//
// SysCallFstat.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallFstat.h"

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

namespace SST { namespace RevCPU {

template<>
template<>
void Fstat<Riscv32>::invoke<int>(Fstat<Riscv32>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fil; 
        stat_t * buf;
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fil);
        has_values[1] = parameters.get<stat_t*>(1, buf);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = fstat(fil, buf);
        }
    }
}

template<>
template<>
void Fstat<Riscv64>::invoke<int>(Fstat<Riscv64>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fil; 
        stat_t * buf;
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fil);
        has_values[1] = parameters.get<stat_t*>(1, buf);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = fstat(fil, buf);
        }
    }
}

template<>
template<>
void Fstat<Riscv128>::invoke<int>(Fstat<Riscv128>::SystemCallParameterInterfaceType & parameters, int & value) {
    if(parameters.count() == 2) {

        int fil; 
        stat_t * buf;
        
        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<int>(0, fil);
        has_values[1] = parameters.get<stat_t*>(1, buf);

        if(has_values[0] && has_values[1]) {
            success = true;
            value = fstat(fil, buf);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST