//
// _RevSysCalls_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVSYSCALLS_H_
#define _SST_REVCPU_REVSYSCALLS_H_

// -- SST Headers
#include "RevInstTable.h"
#include "SST.h"

// -- Standard Headers
#include <array>
#include <string>

// -- RevCPU Headers
#include "../common/include/RevCommon.h"

namespace SST::RevCPU{

enum class EcallStatus{
  SUCCESS = 0,
  CONTINUE = EXCEPTION_CAUSE::ECALL_USER_MODE,
  ERROR = 255,
};

// State information for ECALLs
struct EcallState {
  std::array<char, 64> buf;
  std::string string;
  std::string path_string;
  size_t bytesRead = 0;

  void clear(){
    string.clear();
    path_string.clear();
    bytesRead = 0;
    buf[0] = '\0';
  }
  EcallState() {
    buf[0] = '\0';
  }
};
}
#endif
