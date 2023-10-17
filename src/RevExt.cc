//
// _RevExt_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevExt.h"
using namespace SST::RevCPU;

/// Change the FP rounding mode
auto RevExt::SetFPRound(unsigned Inst, const RevInst& payload, uint16_t HartID, RevRegFile* regFile){
  // Save a copy of the current FP environment
  RevFenv saved_fenv;

  FRMode rm = static_cast<FRMode>(payload.rm);
  switch(rm == FRMode::DYN ? regFile->GetFPRound() : rm){
    case FRMode::RNE:   // Round to Nearest, ties to Even
      RevFenv::SetRound(FE_TONEAREST);
      break;
    case FRMode::RTZ:   // Round towards Zero
      RevFenv::SetRound(FE_TOWARDZERO);
      break;
    case FRMode::RDN:   // Round Down (towards -Inf)
      RevFenv::SetRound(FE_DOWNWARD);
      break;
    case FRMode::RUP:   // Round Up (towards +Inf)
      RevFenv::SetRound(FE_UPWARD);
      break;
    case FRMode::RMM:   // Round to Nearest, ties to Max Magnitude
      output->fatal(CALL_INFO, -1,
                    "Error: Round to nearest Max Magnitude not implemented at index=%u", Inst);
      break;
    case FRMode::DYN:
      output->fatal(CALL_INFO, -1, "Internal Error: DYN mode in fcsr.rm at index=%u",  Inst);
      break;
  }

  return saved_fenv;  // Return saved FP environment state
}

bool RevExt::Execute(unsigned Inst, const RevInst& payload, uint16_t HartID, RevRegFile* regFile){
  bool (*func)(RevFeature *,
               RevRegFile *,
               RevMem *,
               RevInst);

  if( payload.compressed ){
    // this is a compressed instruction, grab the compressed trampoline function
    func = Inst < ctable.size() ? ctable[Inst].func : nullptr;
  }else{
    // retrieve the function pointer for this instruction
    func = Inst < table.size() ? table[Inst].func : nullptr;
  }

  if( !func ){
    output->fatal(CALL_INFO, -1,
                  "Error: instruction at index=%u does not exist in extension=%s",
                  Inst,
                  name.data());
    return false;
  }

  // saved_fenv represents a saved FP environment, which, when
  // destroyed, restores the original FP environment
  auto saved_fenv = SetFPRound(Inst, payload, HartID, regFile);

  // execute the instruction
  bool ret = func(feature, regFile, mem, payload);

  return ret;
}

// EOF
