//
// _RevSysCalls_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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
#include "RevCommon.h"

namespace SST::RevCPU {

enum class EcallStatus {
  SUCCESS,
  CONTINUE,
  ERROR,
};

// State information for ECALLs
struct EcallState {
  std::array<char, 64> buf{};
  std::string          string{};
  std::string          path_string{};
  size_t               bytesRead{};

  void clear() {
    string.clear();
    path_string.clear();
    bytesRead = 0;
  }

  explicit EcallState()                      = default;
  EcallState( const EcallState& )            = delete;
  EcallState& operator=( const EcallState& ) = delete;
};  // struct EcallState
}  // namespace SST::RevCPU
#endif
