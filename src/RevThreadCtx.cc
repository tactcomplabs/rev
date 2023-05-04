//
// _RevThreadCtx_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//


#include "../include/RevThreadCtx.h"
#include <algorithm>
#include <iostream>

bool RevThreadCtx::AddChildPID(uint32_t pid){
  if( std::find(ChildrenPIDs.begin(), ChildrenPIDs.end(), pid) != ChildrenPIDs.end() ){
    ChildrenPIDs.push_back(pid);
    return true;
  }
  else{
    std::cout << "Child process with id = " << pid << "already exists" << std::endl;
    return false;
  }
}

bool RevThreadCtx::RemoveChildPID(uint32_t pid){
  auto ChildToErase = std::find(ChildrenPIDs.begin(), ChildrenPIDs.end(), pid); 
  if (ChildToErase != ChildrenPIDs.end() ){
    ChildrenPIDs.erase(ChildToErase);
    return true;
  }
  else{
    std::cout << "Child process with id = " << pid << "doesn't exist" << std::endl;
    return false;
  }
}
