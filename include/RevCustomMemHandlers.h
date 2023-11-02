//
// _RevCustomMemHandlers_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVCUSTOMMEMHANDLERS_H_
#define _SST_REVCPU_REVCUSTOMMEMHANDLERS_H_

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

// TODO: Add MemReq / flags
inline void ScratchpadHandler(uint64_t Addr, uint64_t Data, size_t Size, void*){ // , const SST::RevCPU::MemReq& req) {
  std::cout << "Scratchpad Handler Called for Address = 0x" << std::hex << Addr << std::endl;
}

#endif
