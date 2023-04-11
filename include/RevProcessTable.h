//
// _RevProcessTable_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once

#include <cstdint>
#ifndef _SST_REVCPU_REVPROCESSTABLE_H_
#define _SST_REVCPU_REVPROCESSTABLE_H_

#include "RevProc.h"
#include "RevProcessTable.h"
#include "RevProcCtx.h"

class RevProcessTable {
public:
      RevProcessTable() :
        PIDCtr(0), ActivePIDs(), InactivePIDs(), Table(), PrevPID(-1), CurrPID(-1) {}

      const static uint32_t PID_MAX = 4194304;
      uint32_t PIDCtr;
      std::vector<uint32_t> ActivePIDs;
      std::vector<uint32_t> InactivePIDs;
      std::map<uint16_t, RevProcCtx> Table;
      uint16_t PrevPID;
      uint16_t CurrPID;

  private:
  void UpdatePIDs(uint16_t pid);
  uint16_t GetPID();
  uint16_t CreateProcess( RevLoader& Loader, RevMem& Mem, RevProc& Proc );
  bool GetProcCtx( uint16_t pid, RevProcCtx& Ctx );
  bool SwitchCtx( RevLoader& Loader, RevMem& Mem, RevProc& Proc, uint16_t FromPID, uint16_t ToPID);
  bool PruneCtx( const uint16_t pid );
  bool RetirePID( uint16_t pid );
}; 

#endif
