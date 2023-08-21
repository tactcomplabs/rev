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

using namespace SST::RevCPU;

/// Used for duplicating register files (currently only in ECALL_clone)
bool RevThreadCtx::DuplicateRegFile(const RevRegFile& regToDup){
  RegFile = regToDup;
  RegFile.RV32_SCAUSE = 0;
  return true;
}
