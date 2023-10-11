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

bool RevExt::Execute(unsigned Inst, const RevInst& payload, unsigned HartID, RevRegFile* regFile){
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
  bool ret = func(feature, regFile, mem, payload);

  return ret;
}

// EOF
