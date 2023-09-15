//
// _RV64F_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64F_H_
#define _SST_REVCPU_RV64F_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

#include <vector>
#include <limits>

namespace SST::RevCPU{

class RV64F : public RevExt {
  static constexpr auto& fcvtls  = CvtFpToInt<float,  int64_t>;
  static constexpr auto& fcvtlus = CvtFpToInt<float, uint64_t>;

  static bool fcvtsl(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP32(F, Inst.rd, static_cast<float>(R->GetX<int64_t>(F, Inst.rs1)));
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool fcvtslu(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP32(F, Inst.rd, static_cast<float>(R->GetX<uint64_t>(F, Inst.rs1)));
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64F Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev64FInstDefaults : RevInstDefaults {
    static constexpr uint8_t     opcode   = 0b1010011;
    static constexpr RevRegClass rdClass  = RegFLOAT;
    static constexpr RevRegClass rs1Class = RegFLOAT;
    static constexpr RevRegClass rs2Class = RegUNKNOWN;
    static constexpr RevRegClass rs3Class = RegUNKNOWN;
  };

  std::vector<RevInstEntry> RV64FTable = {
    {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.l.s  %rd, %rs1").SetFunct7( 0b1100000).SetfpcvtOp(0b00010).Setrs2Class(RegUNKNOWN).SetImplFunc(&fcvtls ).InstEntry},
    {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.lu.s %rd, %rs1").SetFunct7( 0b1100000).SetfpcvtOp(0b00011).Setrs2Class(RegUNKNOWN).SetImplFunc(&fcvtlus ).InstEntry},
    {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.s.l %rd, %rs1" ).SetFunct7( 0b1101000).SetfpcvtOp(0b00010).Setrs2Class(RegUNKNOWN).SetImplFunc(&fcvtsl ).InstEntry},
    {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.s.lu %rd, %rs1").SetFunct7( 0b1101000).SetfpcvtOp(0b00011).Setrs2Class(RegUNKNOWN).SetImplFunc(&fcvtslu ) .InstEntry},
  };


public:
  /// RV364F: standard constructor
  RV64F( RevFeature *Feature,
         RevRegFile *RegFile,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV64F", Feature, RegFile, RevMem, Output) {
    SetTable(RV64FTable);
  }

  /// RV64F: standard destructor
  ~RV64F() = default;

}; // end class RV64F

} // namespace SST::RevCPU

#endif
