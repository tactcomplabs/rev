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

  bool GetCommand();
  SST::Cycle_t GetNextBreakpoint(){return breakCycle;};
  void SetNextBreakpoint(SST::Cycle_t cycle){ breakCycle = cycle;}

  void SetProcToDebug(RevProc* p){proc = p;};

  protected:
  SST::Cycle_t breakCycle;
  RevProc* proc;
  std::vector<std::string> cmdLine;

  void PrintRegister();
  void Step();
  void PrintHelp();

};

};


#endif