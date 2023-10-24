//
// _RevThread_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#include "RevThread.h"

namespace SST::RevCPU{

std::ostream& operator<<(std::ostream& os, const RevThread& Thread){
  os << "\n";

  // Calculate total width of the table
  int tableWidth = 6 /*Reg*/ + 7 /*Alias*/ + 16 /*Value*/ + 23 /*Info*/ + 9 /*Separators*/;

  // Print a top border
  os << "|" << std::string(tableWidth-1, '=') << "|" << '\n';

  // Print Thread ID
  os << "| Thread " << Thread.GetThreadID() << std::setw(6) <<  std::string(tableWidth-10, ' ') << "|\n";

  // Print the middle border
  os << "|" << std::string(tableWidth-1, '-') << "|" << '\n';

  std::string StateString = "";
  switch (Thread.GetState()){
  case ThreadState::START:
    StateString = "START";
    break;
  case ThreadState::READY:
    StateString = "READY";
    break;
  case ThreadState::RUNNING:
    StateString = "RUNNING";
    break;
  case ThreadState::BLOCKED:
    StateString = "BLOCKED";
    break;
  case ThreadState::DONE:
    StateString = "DONE";
    break;
  default:
    StateString = "UNKNOWN";
    break;
  }
  // Print a nice header
  os << " ==> State: " << StateString << "\n";
  os << " ==> ParentTID: " << Thread.GetParentThreadID() << "\n";
  os << " ==> Blocked by TID: " ;
  if (Thread.GetWaitingToJoinTID() != _INVALID_TID_) {
    os << Thread.GetWaitingToJoinTID();
  } else {
    os << "N/A";
  }
  os << "\n";

  os << Thread.RegFile;

  return os;
}

} // namespace SST::RevCPU
