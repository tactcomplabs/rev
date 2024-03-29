//
// _RV64P_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64P_H_
#define _SST_REVCPU_RV64P_H_

#include "../RevInstHelpers.h"

#include <vector>

namespace SST::RevCPU{

class RV64P : public RevExt{

  static bool future(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetX(Inst.rd, !!M->SetFuture(R->GetX<uint64_t>(Inst.rs1) + Inst.ImmSignExt(12)));
    return true;
  }

  static bool rfuture(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetX(Inst.rd, !!M->RevokeFuture(R->GetX<uint64_t>(Inst.rs1) + Inst.ImmSignExt(12)));
    return true;
  }

  static bool sfuture(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetX(Inst.rd, !!M->StatusFuture(R->GetX<uint64_t>(Inst.rs1) + Inst.ImmSignExt(12)));
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64P Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev64PInstDefaults : RevInstDefaults {
    static constexpr uint8_t     opcode   = 0b1110111;
    static constexpr RevRegClass rs2Class = RevRegClass::RegUNKNOWN;
    static constexpr RevImmFunc  imm      = FImm;
    static constexpr RevInstF    format   = RVTypeI;
  };

  std::vector<RevInstEntry> RV64PTable = {
    {RevInstEntryBuilder<Rev64PInstDefaults>().SetMnemonic("future %rd, $imm(%rs1)" ).SetFunct3(0b111).SetImplFunc(&future).InstEntry},
    {RevInstEntryBuilder<Rev64PInstDefaults>().SetMnemonic("rfuture %rd, $imm(%rs1)").SetFunct3(0b101).SetImplFunc(&rfuture).InstEntry},
    {RevInstEntryBuilder<Rev64PInstDefaults>().SetMnemonic("sfuture %rd, $imm(%rs1)").SetFunct3(0b100).SetImplFunc(&sfuture).InstEntry},
  };

public:
  /// RV64P: standard constructor
  RV64P( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV64P", Feature, RevMem, Output ) {
    SetTable(std::move(RV64PTable));
  }
}; // end class RV64I

} // namespace SST::RevCPU

#endif
