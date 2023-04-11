//
// _RevProcessTable_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevProcessTable.h"


uint16_t RevProcessTable::GetPID(){
  // check size of inactive pids
  if( InactivePIDs.size() < 1 ){
    if( ActivePIDs.size() > 0 ){
      // Find biggest existing & active pid
      auto MaxPID = std::max_element(ActivePIDs.begin(), ActivePIDs.end());
      uint32_t NewPID = *MaxPID + 1;
      ActivePIDs.push_back(NewPID);
      return (uint16_t)NewPID;
    }
    // No active PIDs 
    ActivePIDs.push_back(0);
    return 0;
  }
  // Inactive PIDs 
  uint32_t NewPID = InactivePIDs.at(InactivePIDs.size()-1);
  InactivePIDs.pop_back();
  return NewPID;
}

bool RevProcessTable::RetirePID(uint16_t pid){
  auto pid_iter = std::find(ActivePIDs.begin(), ActivePIDs.end(), pid);
  if( pid_iter != std::end(ActivePIDs) ){
    // move active pid to inactive queue
    InactivePIDs.push_back(*pid_iter);
    ActivePIDs.erase(pid_iter);
    return true;
  }
  return false;
}

uint16_t RevProcessTable::CreateProcess( RevLoader& loader,RevMem& Mem, RevProc& Proc ){
  // Get active pid
  const uint16_t pid = GetPID();
  RevProcCtx C {Proc.GetThreadID(),
                pid,
                loader.GetSymbolAddr("main"),
                Proc.RegFile[Proc.GetThreadID()],
                0};
  Proc.SaveCtx(Mem, C);
  Table.emplace(std::make_pair(pid, std::move(C)));
  return pid;
}

// bool GetCurrProc(RevProc& Proc){
//   std::map<uint16_t, RevProcCtx>::iterator CurrProcCtx = Table.find(CurrPID);
//   if( CurrProcCtx != std::end(Table) ){
//     Proc = CurrProcCtx->second;
//     return true;
//   }
//   return false;
// }

bool RevProcessTable::GetProcCtx(uint16_t pid, RevProcCtx& Ctx){
  std::map<uint16_t, RevProcCtx>::iterator PrevProcCtx = Table.find(pid);
  if( PrevProcCtx != std::end(Table) ){
    Ctx = PrevProcCtx->second;
    return true;
  }
  return false;
}

// Table will be a map of <pid, Ctx>

bool RevProcessTable::SwitchCtx( RevLoader& loader, RevMem& Mem, RevProc& Proc, 
                                 uint16_t FromPID, uint16_t ToPID){
  
  if( ActivePIDs.find(FromPID) != ActivePIDs.end() 
      && ActivePIDs.find(ToPID) != ActivePIDs.end() ){
    // deactivate FromCtx PID
    RetirePID(FromPID);
    PrevPID = CurrPID;
    CurrPID = ToPID;
    RevProcCtx C {};
    Table.GetProcCtx(ToPID, C);
    Proc.LoadCtx(Mem, C);
    return true;
  }
  return false;
}

bool RevProcessTable::PruneCtx(const uint16_t pid){
  auto itr = Table.find(pid);  
  if( itr == std::end(Table) ){
    return false;
  }
  Table.erase(itr);
  return true;
}
