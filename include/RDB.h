//
// _RDB_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef __REV_RDB_H__
#define __REV_RDB_H__

#include <iostream>

// -- SST Headers
#include "SST.h"

//Rev Headers
#include "RevProc.h"

namespace SST::RevCPU{
class RevProc;

class RDB {
  public:
  RDB(SST::Cycle_t firstBreak);
  ~RDB();

  void GetCommand();
  SST::Cycle_t GetNextBreakpoint(){return breakCycle;};
  void SetNextBreakpoint(SST::Cycle_t breakCycle){ breakCycle = breakCycle;}

  void SetProcToDebug(RevProc* proc){proc = proc;};

  protected:
  SST::Cycle_t breakCycle;
  RevProc* proc;
};

};


#endif