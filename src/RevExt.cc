//
// _RevExt_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevExt.h"

namespace SST::RevCPU {

/// Execute an instruction
bool RevExt::Execute( unsigned Inst, const RevInst& payload, uint16_t HartID, RevRegFile* regFile ) {
  bool ( *func )( RevFeature*, RevRegFile*, RevMem*, const RevInst& );

  if( payload.compressed ) {
    // this is a compressed instruction, grab the compressed trampoline function
    func = Inst < ctable.size() ? ctable[Inst].func : nullptr;
  } else {
    // retrieve the function pointer for this instruction
    func = Inst < table.size() ? table[Inst].func : nullptr;
  }

  if( !func ) {
    output->fatal( CALL_INFO, -1, "Error: instruction at index=%u does not exist in extension=%s", Inst, name.data() );
    return false;
  }

  bool ret = false;
  if( !payload.raisefpe ) {
    // If the instruction cannot raise FP exceptions, don't mess with the FP
    // environment. No RISC-V FP instructions which cannot raise FP exceptions
    // depend on the FP rounding mode, i.e. depending on rounding mode implies
    // that the instruction can raise FP exceptions.
    ret = func( feature, regFile, mem, payload );
  } else {
    // saved_fenv represents a saved FP environment on the host, which, when
    // destroyed, restores the original FP host environment. We execute the
    // instruction in a default FP environment on the host with all FP
    // exceptions cleared and the rounding mode set according to the encoding
    // and frm register. The FP exceptions which occur on the host are stored
    // in FCSR when saved_fenv is destroyed.
    RevFenv saved_fenv( regFile, payload.rm, output );

    // Execute the instruction
    ret = func( feature, regFile, mem, payload );

    // Fall-through destroys saved_fenv, setting the FCSR's fflags and
    // restoring the host's FP environment
  }

  return ret;
}

}  // namespace SST::RevCPU

// EOF
