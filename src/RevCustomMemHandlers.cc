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

inline void ScratchpadHandler( struct RevMem::CustomMemArgs Args){
  if( Args.IsWrite ){
    std::cout << "Scratchpad WRITE Handler Called By Hart " << Args.HartID << " for Address = 0x" << std::hex << Args.Addr << " with Data = 0x" << Args.Data << std::endl;
  }else{
    std::cout << "Scratchpad READ Handler Called By Hart " << Args.HartID << " for Address = 0x" << std::hex << Args.Addr << std::endl;
  }
}

inline void Mem1Handler( struct RevMem::CustomMemArgs Args){
  if( Args.IsWrite ){
    std::cout << "Mem1 WRITE Handler Called By Hart " << Args.HartID << " for Address = 0x" << std::hex << Args.Addr << " with Data = 0x" << Args.Data << std::endl;
  }else{
    std::cout << "Mem1 READ Handler Called By Hart " << Args.HartID << " for Address = 0x" << std::hex << Args.Addr << std::endl;
  }
}

inline void Mem2Handler( struct RevMem::CustomMemArgs Args){
  if( Args.IsWrite ){
    std::cout << "Mem2 WRITE Handler Called By Hart " << Args.HartID << " for Address = 0x" << std::hex << Args.Addr << " with Data = 0x" << Args.Data << std::endl;
  }else{
    std::cout << "Mem2 READ Handler Called By Hart " << Args.HartID << " for Address = 0x" << std::hex << Args.Addr << std::endl;
  }
}
std::unordered_map<std::string, std::function<void(struct RevMem::CustomMemArgs)>> RevMem::CustomMemHandlers = {
  {"scratchpad", &ScratchpadHandler},
  {"mem1", &Mem1Handler},
  {"mem2", &Mem2Handler},
};


