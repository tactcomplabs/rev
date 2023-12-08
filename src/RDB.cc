
//
// _RDB_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RDB.h"


namespace SST::RevCPU{
RDB::RDB(SST::Cycle_t firstBreak):
  breakCycle(firstBreak) {

  proc = nullptr;
  
}

RDB::~RDB(){

}

void RDB::GetCommand(){

  std::cout << "Core 0 PC: " << "rdb% ";
  std::cin >> breakCycle;

}

}