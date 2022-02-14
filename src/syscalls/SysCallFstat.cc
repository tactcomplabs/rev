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

namespace SST { namespace RevCPU {

template<>
template<>
bool FstatParameters<Riscv32>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool FstatParameters<Riscv64>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool FstatParameters<Riscv128>::get<int>(const size_t parameter_index, int& param) {
    if(parameter_index == 0) {
        param = fildes;
        return true;
    }

    return false;
}

template<>
template<>
bool FstatParameters<Riscv32>::get<stat_t*>(const size_t parameter_index, stat_t * & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<>
template<>
bool FstatParameters<Riscv64>::get<stat_t*>(const size_t parameter_index, stat_t * & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<>
template<>
bool FstatParameters<Riscv128>::get<stat_t*>(const size_t parameter_index, stat_t * & param) {
    if(parameter_index == 1) {
        param = buf;
        return true;
    }

    return false;
}

template<>
template<>
void Fstat<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
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
void Fstat<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
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
void Fstat<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
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
