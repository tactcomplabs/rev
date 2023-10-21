//
// _RevHartManager_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevHartManager.h"
using namespace SST::RevCPU;

#include "SST.h"

  // Get next hart to decode
std::shared_ptr<RevHart>& RevHartManager::NextHartToDecodeID(){
  // TODO:
  return Harts[0];
}

// Update Harts to decode
void RevHartManager::UpdateHartsToDecode(){
  return;
}

// Checks if a hart that decoded is ready to execute
bool RevHartManager::CanHartExecute(unsigned HartID) const {
  // We can assume there is a thread because only BusyHarts get
  // selected to decode
  return (Harts.at(HartID)->GetRegFile()->GetCost() == 0 );
}

// Pop thread from Hart and return the thread
std::unique_ptr<RevThread> RevHartManager::PopThreadFromHart(unsigned HartID){
  // TODO: Add error handling
  BusyHarts.erase(HartID);
  IdleHarts.insert(HartID);
  return Harts.at(HartID)->PopThread();
}




