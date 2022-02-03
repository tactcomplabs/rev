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

template<>
template<>
void MremapSystemCall<Riscv32>::invoke<void_t>(MremapSystemCall<Riscv32>::SystemCallParameterInterfaceType & parameters, void_t & value) {

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
void MremapSystemCall<Riscv64>::invoke<void_t>(MremapSystemCall<Riscv64>::SystemCallParameterInterfaceType & parameters, void_t & value) {

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
void MremapSystemCall<Riscv128>::invoke<void_t>(MremapSystemCall<Riscv128>::SystemCallParameterInterfaceType & parameters, void_t & value) {

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