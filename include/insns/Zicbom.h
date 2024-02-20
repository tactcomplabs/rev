//
// _Zicbom_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZICBOM_H_
#define _SST_REVCPU_ZICBOM_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>
#include <limits>

namespace SST::RevCPU{
#define CBO_INVAL_IMM 0b000000000000
#define CBO_FLUSH_IMM 0b000000000001
#define CBO_CLEAN_IMM 0b000000000010
class Zicbom : public RevExt{

  static bool cmo(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    switch(Inst.imm){
    case CBO_INVAL_IMM:
      // CBO.INVAL
      M->InvLine(F->GetHartToExecID(), R->GetX<uint64_t>(Inst.rs1));
      break;
    case CBO_FLUSH_IMM:
      // CBO.FLUSH
      M->FlushLine(F->GetHartToExecID(), R->GetX<uint64_t>(Inst.rs1));
      break;
    case CBO_CLEAN_IMM:
      // CBO.CLEAN
      M->CleanLine(F->GetHartToExecID(), R->GetX<uint64_t>(Inst.rs1));
      break;
    default:
      return false;
    }
    R->AdvancePC(Inst);
    return true;
  }

  struct RevZicbomInstDefaults : RevInstDefaults {
    RevZicbomInstDefaults(){
      SetFormat(RVTypeI);
      SetOpcode(0b0001111);
      SetFunct3(0b010);
      Setrs2Class(RevRegClass::RegUNKNOWN);
      SetrdClass(RevRegClass::RegUNKNOWN);
      Setimm(FEnc);
      SetImplFunc(cmo);
    }
  };

  std::vector<RevInstEntry> ZicbomTable = {
    { RevZicbomInstDefaults().SetMnemonic("cbo.clean").Setimm12(0b0001) },
    { RevZicbomInstDefaults().SetMnemonic("cbo.flush").Setimm12(0b0010) },
    { RevZicbomInstDefaults().SetMnemonic("cbo.inval").Setimm12(0b0000) },
  };

public:
  Zicbom( RevFeature *Feature,
          RevMem *RevMem,
          SST::Output *Output )
    : RevExt( "Zicbom", Feature, RevMem, Output){
    SetTable(std::move(ZicbomTable));
  }
};  // end class Zicbom
}   // namespace SST::RevCPU

#endif
