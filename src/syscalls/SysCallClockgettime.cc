//
// SysCallClockgettime.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCallClockgettime.h"

namespace SST { namespace RevCPU {

template<>
template<>
bool ClockgettimeParameters<Riscv32>::get<clockid_t>(const size_t parameter_index, clockid_t& param) {
    if(parameter_index == 0) {
        param = clkid;
        return true;
    }

    return false;
}

template<>
template<>
bool ClockgettimeParameters<Riscv64>::get<clockid_t>(const size_t parameter_index, clockid_t& param) {
    if(parameter_index == 0) {
        param = clkid;
        return true;
    }

    return false;
}

template<>
template<>
bool ClockgettimeParameters<Riscv128>::get<clockid_t>(const size_t parameter_index, clockid_t& param) {
    if(parameter_index == 0) {
        param = clkid;
        return true;
    }

    return false;
}

template<>
template<>
bool ClockgettimeParameters<Riscv32>::get<timespec*>(const size_t parameter_index, timespec* & param) {
   if(parameter_index == 1) {
       param = tp;
       return true;
   }

   return false;
}

template<>
template<>
bool ClockgettimeParameters<Riscv64>::get<timespec*>(const size_t parameter_index, timespec* & param) {
   if(parameter_index == 1) {
       param = tp;
       return true;
   }

   return false;
}

template<>
template<>
bool ClockgettimeParameters<Riscv128>::get<timespec*>(const size_t parameter_index, timespec* & param) {
   if(parameter_index == 1) {
       param = tp;
       return true;
   }

   return false;
}

template<>
template<>
void Clockgettime<Riscv32>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parameters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

template<>
template<>
void Clockgettime<Riscv64>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parameters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

template<>
template<>
void Clockgettime<Riscv128>::invoke<int>(RevRegFile const& memregfile, RevMem const& revmemory, int & value) {
    if(parameters.count() == 2) {
        clockid_t clkid;
        timespec * tp;

        bool has_values[2] = { false, false };
        has_values[0] = parameters.get<clockid_t>(0, clkid);
        has_values[1] = parameters.get<timespec*>(1, tp);
        success = true;
        value = clock_gettime(clkid, tp);
    }
}

} /* end namespace RevCPU */ } // end namespace SST
