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

bool RevExt::Execute(unsigned Inst, const RevInst& payload, uint16_t HartID, RevRegFile* regFile){
  bool (*func)(RevFeature *,
               RevRegFile *,
               RevMem *,
               RevInst) = nullptr;
  try{
    if( payload.compressed ){
      // this is a compressed instruction, grab the compressed trampoline function
      func = ctable.at(Inst).func;
    }else{
      // retrieve the function pointer for this instruction
      func = table.at(Inst).func;
    }
  } catch(...) {
    output->fatal(CALL_INFO, -1,
                  "Error: instruction at index=%u does not exist in extension=%s",
                  Inst,
                  name.data());
    throw;
  }

  // execute the instruction
  bool ret = func(feature, regFile, mem, payload);

  return ret;
}

// EOF
