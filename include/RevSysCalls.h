//
// SysCalls.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLS_H__
#define __SYSTEMCALLS_H__


#include <unordered_map>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <utility>
#include <unistd.h>
#include <filesystem>
#include <variant>
#include "RevInstTable.h"
#include "RevSysCallInterface.h"
#include "RevMem.h"

namespace SST { namespace RevCPU {

// using return_t = std::variant<
//   std::monostate, // -- null case
//   int,
//   ssize_t,
//   pid_t,
//   timeval
// >;

class SystemCalls {
    public:
    SystemCalls() = default;

    // static std::unordered_map< int, std::pair<systemcall_t, return_t>> jump_table32; //-- 32-bit SysCalls
    // static std::unordered_map< int, std::pair<systemcall_t, return_t>> jump_table64; //-- 64-bit SysCalls

    static std::unordered_map<int, systemcall_t> jump_table32; //-- 32-bit SysCalls
    static std::unordered_map<int, systemcall_t> jump_table64; //-- 64-bit SysCalls
};

} /* end namespace RevCPU */ } // end namespace SST

#endif
// EOF
