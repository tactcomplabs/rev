//
// SysCallMmap.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallMremap.h"
#include <stdlib.h>

namespace SST { namespace RevCPU {


template<typename RiscvArchType>
template<>
bool MremapParameters<RiscvArchType>::get(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 0) {
        param = oldaddr;
        return true;
    }
    else if(parameter_index == 4 && (newaddr == std::nullptr) ) {
        param = newaddr;
        return true;
    }

    return false;
}

template<typename RiscvArchType>
template<>
bool MremapParameters<RiscvArchType>::get(const size_t parameter_index, int & param) {
    if(parameter_index == 3) {
        param = flags;
        return true;
    }

        return false;
}

template<typename RiscvArchType>
template<>
bool MremapParameters<RiscvArchType>::get(const size_t parameter_index, size_t & param) {
    if(parameter_index == 1) {
        param = oldsize;
        return true;
    }
    else if(parameter_index == 2) {
        param = newsize;
        return true;
    }

    return false;
}

template<>
template<>
void Mremap<Riscv32>::invoke<void_t>(Mremap<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value) {

    if(parameters.count() > 4 && parameters.count() < 7) {
        void * oldaddr;
        size_t oldlen;
        size_t newlen;
        int flags;
        void * newaddr;
        
        bool hasargs[5] = { false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, oldaddr);
        hasargs[1] = parameters.get<size_t>(1, oldlen);
        hasargs[2] = parameters.get<size_t>(2, newlen);
        hasargs[3] = parameters.get<int>(3, flags);
        hasargs[4] = parameters.get<void_ptr>(4, newaddr);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4]) {
            success = true;
            value = mremap(oldaddr, oldlen, newlen, flags, newaddr);
        }
        else if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3]) {
            success = true;
            value = mremap(oldaddr, oldlen, newlen, flags);
        }        
    }
}

template<>
template<>
void Mremap<Riscv64>::invoke<void_t>(Mremap<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value) {

    if(parameters.count() > 4 && parameters.count() < 7) {
        void * oldaddr;
        size_t oldlen;
        size_t newlen;
        int flags;
        void * newaddr;
        
        bool hasargs[5] = { false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, oldaddr);
        hasargs[1] = parameters.get<size_t>(1, oldlen);
        hasargs[2] = parameters.get<size_t>(2, newlen);
        hasargs[3] = parameters.get<int>(3, flags);
        hasargs[4] = parameters.get<void_ptr>(4, newaddr);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4]) {
            success = true;
            value = mremap(oldaddr, oldlen, newlen, flags, newaddr);
        }
        else if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3]) {
            success = true;
            value = mremap(oldaddr, oldlen, newlen, flags);
        }        
    }
}

template<>
template<>
void Mremap<Riscv128>::invoke<void_t>(Mremap<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value) {

    if(parameters.count() > 4 && parameters.count() < 7) {
        void * oldaddr;
        size_t oldlen;
        size_t newlen;
        int flags;
        void * newaddr;
        
        bool hasargs[5] = { false, false, false, false, false };

        hasargs[0] = parameters.get<void_ptr>(0, oldaddr);
        hasargs[1] = parameters.get<size_t>(1, oldlen);
        hasargs[2] = parameters.get<size_t>(2, newlen);
        hasargs[3] = parameters.get<int>(3, flags);
        hasargs[4] = parameters.get<void_ptr>(4, newaddr);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3] && hasargs[4]) {
            success = true;
            value = mremap(oldaddr, oldlen, newlen, flags, newaddr);
        }
        else if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3]) {
            success = true;
            value = mremap(oldaddr, oldlen, newlen, flags);
        }        
    }
}

} /* end namespace RevCPU */ } // end namespace SST
