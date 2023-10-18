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

/// Change the FP environment
auto RevExt::SetFPEnv(unsigned Inst, const RevInst& payload, uint16_t HartID, RevRegFile* regFile){
  // Save a copy of the current FP environment
  RevFenv saved_fenv;

  switch(payload.rm == FRMode::DYN ? regFile->GetFPRound() : payload.rm){
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
                    "Error: Round to nearest Max Magnitude not implemented at PC = 0x%" PRIx64 "\n",
                    regFile->GetPC());
      break;
    default:
      output->fatal(CALL_INFO, -1, "Illegal instruction: Bad FP rounding mode at PC = 0x%" PRIx64 "\n",
                    regFile->GetPC());
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

  // execute the instruction
  bool ret = false;

  if(payload.rm == FRMode::None){
    // If the instruction has no FRMode, we do not need to save and restore it
    ret = func(feature, regFile, mem, payload);
  }else{
    // saved_fenv represents a saved FP environment, which, when destroyed,
    // restores the original FP environment. We execute the function in the
    // modified FP environment on the host, then restore the FP environment.
    auto saved_fenv = SetFPEnv(Inst, payload, HartID, regFile);
    ret = func(feature, regFile, mem, payload);
  }

  return ret;
}

// EOF
