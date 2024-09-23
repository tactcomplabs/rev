//
// _RevFCSR_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVFCSR_H_
#define _SST_REVFCSR_H_

#include <cstdint>

namespace SST::RevCPU {

/// Floating-Point Rounding Mode
enum class FRMode : uint32_t {
  None = 0xff,
  RNE  = 0,  // Round to Nearest, ties to Even
  RTZ  = 1,  // Round towards Zero
  RDN  = 2,  // Round Down (towards -Inf)
  RUP  = 3,  // Round Up (towards +Inf)
  RMM  = 4,  // Round to Nearest, ties to Max Magnitude
  DYN  = 7,  // In instruction's rm field, selects dynamic rounding mode; invalid in FCSR
};

/// Floating-point control register
enum class FCSR : uint32_t {
  NX = 1,
  UF = 2,
  OF = 4,
  DZ = 8,
  NV = 16,
};

}  // namespace SST::RevCPU

#endif
