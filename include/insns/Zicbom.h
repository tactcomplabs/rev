//
// _Zicbom_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
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
      break;
    }
    R->AdvancePC(Inst);
    return true;
  }

  struct RevZicbomInstDefaults : RevInstDefaults {
    static constexpr uint8_t     opcode   = 0b0001111;
    static constexpr uint8_t     funct3   = 0b010;
    static constexpr RevRegClass rs1Class = RevRegClass::RegGPR;
    static constexpr RevRegClass rs2Class = RevRegClass::RegUNKNOWN;
    static constexpr RevRegClass rs3Class = RevRegClass::RegUNKNOWN;
    static constexpr RevRegClass rdClass  = RevRegClass::RegUNKNOWN;
  };

  std::vector<RevInstEntry> ZicbomTable = {
    {RevInstEntryBuilder<RevZicbomInstDefaults>().SetMnemonic("cbo.clean").SetCost(1).Setimm12(0b000000000001).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&cmo).InstEntry},
    {RevInstEntryBuilder<RevZicbomInstDefaults>().SetMnemonic("cbo.flush").SetCost(1).Setimm12(0b000000000010).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&cmo).InstEntry},
    {RevInstEntryBuilder<RevZicbomInstDefaults>().SetMnemonic("cbo.inval").SetCost(1).Setimm12(0b000000000000).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&cmo).InstEntry},
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
