//
// SysCallPrlimit64.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallPrlimit64.h"
#include <stdlib.h>

namespace SST { namespace RevCPU {

template<>
template<>
bool Prlimit64Parameters<Riscv32>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 2) {
        param = old_limit;
        return true;
    }
    else if(parameter_index == 3) {
        param = new_limit;
        return true;
    }

    return false;
}

template<>
template<>
bool Prlimit64Parameters<Riscv64>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 2) {
        param = old_limit;
        return true;
    }
    else if(parameter_index == 3) {
        param = new_limit;
        return true;
    }

    return false;
}

template<>
template<>
bool Prlimit64Parameters<Riscv128>::get<void_ptr>(const size_t parameter_index, void_ptr & param) {
    if(parameter_index == 2) {
        param = old_limit;
        return true;
    }
    else if(parameter_index == 3) {
        param = new_limit;
        return true;
    }

    return false;
}

template<>
template<>
bool Prlimit64Parameters<Riscv32>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }
    else if(parameter_index == 1) {
        param = resource;
        return true;
    }

    return false;
}

template<>
template<>
bool Prlimit64Parameters<Riscv64>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }
    else if(parameter_index == 1) {
        param = resource;
        return true;
    }

    return false;
}

template<>
template<>
bool Prlimit64Parameters<Riscv128>::get<int>(const size_t parameter_index, int & param) {
    if(parameter_index == 0) {
        param = pid;
        return true;
    }
    else if(parameter_index == 1) {
        param = resource;
        return true;
    }

    return false;
}

template<>
template<>
void Prlimit64<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 4) {

        pid_t pid; 
        int resource;
        rlimit_t *new_limit;
        rlimit_t *old_limit;

        bool hasargs[4] = { false, false, false, false };

        hasargs[0] = parameters.get<pid_t>(0, pid);
        hasargs[1] = parameters.get<int>(1, resource);
        hasargs[2] = parameters.get<rlimit_t *>(2, new_limit);
        hasargs[3] = parameters.get<rlimit_t *>(3, old_limit);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3]) {
            success = true;
            value = prlimit64(pid, (__rlimit_resource)resource, (const rlimit64*)new_limit, old_limit);
        }
    }
}

template<>
template<>
void Prlimit64<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 4) {

        pid_t pid; 
        int resource;
        rlimit_t *new_limit;
        rlimit_t *old_limit;

        bool hasargs[4] = { false, false, false, false };

        hasargs[0] = parameters.get<pid_t>(0, pid);
        hasargs[1] = parameters.get<int>(1, resource);
        hasargs[2] = parameters.get<rlimit_t *>(2, new_limit);
        hasargs[3] = parameters.get<rlimit_t *>(3, old_limit);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3]) {
            success = true;
            value = prlimit64(pid, (__rlimit_resource)resource, (const rlimit64*)new_limit, old_limit);
        }
    }
}

template<>
template<>
void Prlimit64<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {

    if(parameters.count() == 4) {

        pid_t pid; 
        int resource;
        rlimit_t *new_limit;
        rlimit_t *old_limit;

        bool hasargs[4] = { false, false, false, false };

        hasargs[0] = parameters.get<pid_t>(0, pid);
        hasargs[1] = parameters.get<int>(1, resource);
        hasargs[2] = parameters.get<rlimit_t *>(2, new_limit);
        hasargs[3] = parameters.get<rlimit_t *>(3, old_limit);

        if(hasargs[0] && hasargs[1] && hasargs[2] && hasargs[3]) {
            success = true;
            value = prlimit64(pid, (__rlimit_resource)resource, (const rlimit64*)new_limit, old_limit);
        }
    }
}

} /* end namespace RevCPU */ } // end namespace SST
