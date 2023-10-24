//
// _RV64D_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64D_H_
#define _SST_REVCPU_RV64D_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>
#include <cstring>

namespace SST::RevCPU{

class RV64D : public RevExt {
  static constexpr auto& fcvtld  = CvtFpToInt<double,  int64_t>;
  static constexpr auto& fcvtlud = CvtFpToInt<double, uint64_t>;

  static bool fcvtdl(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, static_cast<double>(R->GetX<int64_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtdlu(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, static_cast<double>(R->GetX<uint64_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmvxd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    uint64_t u64;
    double fp = R->GetFP<double>(Inst.rs1);
    memcpy(&u64, &fp, sizeof(u64));
    R->SetX(Inst.rd, u64);
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmvdx(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    uint64_t u64 = R->GetX<uint64_t>(Inst.rs1);
    double fp;
    memcpy(&fp, &u64, sizeof(fp));
    R->SetFP(Inst.rs1, fp);
    R->AdvancePC(Inst);
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64D Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev64DInstDefaults : RevInstDefaults {
    static constexpr uint8_t     opcode   = 0b1010011;
    static constexpr RevRegClass rdClass  = RevRegClass::RegFLOAT;
    static constexpr RevRegClass rs1Class = RevRegClass::RegFLOAT;
    static constexpr RevRegClass rs2Class = RevRegClass::RegUNKNOWN;
  };

  std::vector<RevInstEntry> RV64DTable = {
    {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.l.d %rd, %rs1"  ).SetFunct7(0b1100001).SetfpcvtOp(0b10).SetImplFunc( &fcvtld ).InstEntry},
    {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.lu.d %rd, %rs1" ).SetFunct7(0b1100001).SetfpcvtOp(0b11).SetImplFunc( &fcvtlud ).InstEntry},
    {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.d.l %rd, %rs1"  ).SetFunct7(0b1101001).SetfpcvtOp(0b10).SetImplFunc( &fcvtdl ).InstEntry},
    {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.d.lu %rd, %rs1" ).SetFunct7(0b1101001).SetfpcvtOp(0b11).SetImplFunc( &fcvtdlu ).InstEntry},
    {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fmv.x.d %rd, %rs1"   ).SetFunct7(0b1110001).SetImplFunc( &fmvxd ).InstEntry},
    {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fmv.d.x %rd, %rs1"   ).SetFunct7(0b1111001).SetImplFunc( &fmvdx ).InstEntry},
  };

public:
  /// RV364D: standard constructor
  RV64D( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV64D", Feature, RevMem, Output) {
    SetTable(std::move(RV64DTable));
  }
}; // end class RV32I

} // namespace SST::RevCPU

#endif
