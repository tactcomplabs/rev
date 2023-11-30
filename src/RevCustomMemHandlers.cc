//
// _RevCustomMemHandlers_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

// -- C++ Headers
#include <algorithm>
#include <cstdio>
#include <iostream>

// -- SST Headers
#include "SST.h"

// -- RevCPU Headers
#include "RevOpts.h"
#include "RevMemCtrl.h"
#include "RevTracer.h"
#include "RevRand.h"
#include "RevCommon.h"
#include "RevMem.h"

using namespace SST::RevCPU;

// TODO: Add MemReq / flags
inline void ScratchpadHandler(unsigned HartID, uint64_t Addr, uint64_t Data, size_t Size){//, void*){ // , const SST::RevCPU::MemReq& req) {
  std::cout << "Scratchpad Handler Called By Hart " << HartID << " for Address = 1x" << std::hex << Addr << std::endl;
}

// TODO: Add MemReq to this
std::unordered_map<std::string, std::function<void(unsigned,
                                                   uint64_t,
                                                   uint64_t,
                                                   size_t)>> RevMem::CustomMemHandlers = {
  {"scratchpad", &ScratchpadHandler},
};
