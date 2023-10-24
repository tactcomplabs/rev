//
// _RV32D_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32D_H_
#define _SST_REVCPU_RV32D_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>
#include <cmath>

namespace SST::RevCPU{

class RV32D : public RevExt{

  // Compressed instructions
  static bool cfldsp(RevFeature *F, RevRegFile *R,
                     RevMem *M, RevInst Inst) {
    // c.flwsp rd, $imm = lw rd, x2, $imm
    Inst.rs1  = 2;
    return fld(F, R, M, Inst);
  }

  static bool cfsdsp(RevFeature *F, RevRegFile *R,
                     RevMem *M, RevInst Inst) {
    // c.fsdsp rs2, $imm = fsd rs2, x2, $imm
    Inst.rs1  = 2;
    return fsd(F, R, M, Inst);
  }

  static bool cfld(RevFeature *F, RevRegFile *R,
                   RevMem *M, RevInst Inst) {
    // c.fld %rd, %rs1, $imm = flw %rd, %rs1, $imm
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = CRegIdx(Inst.rs1);
    return fld(F, R, M, Inst);
  }

  static bool cfsd(RevFeature *F, RevRegFile *R,
                   RevMem *M, RevInst Inst) {
    // c.fsd rs2, rs1, $imm = fsd rs2, $imm(rs1)
    Inst.rs2 = CRegIdx(Inst.rd);
    Inst.rs1 = CRegIdx(Inst.rs1);
    return fsd(F, R, M, Inst);
  }

  // Standard instructions
  static constexpr auto& fld = fload<double>;
  static constexpr auto& fsd = fstore<double>;

  static bool fmaddd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, std::fma(R->GetFP<double>(Inst.rs1), R->GetFP<double>(Inst.rs2), R->GetFP<double>(Inst.rs3)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmsubd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, std::fma(R->GetFP<double>(Inst.rs1), R->GetFP<double>(Inst.rs2), -R->GetFP<double>(Inst.rs3)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fnmsubd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, std::fma(-R->GetFP<double>(Inst.rs1), R->GetFP<double>(Inst.rs2), R->GetFP<double>(Inst.rs3)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fnmaddd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, -std::fma(R->GetFP<double>(Inst.rs1), R->GetFP<double>(Inst.rs2), R->GetFP<double>(Inst.rs3)));
    R->AdvancePC(Inst);
    return true;
  }

  static constexpr auto& faddd = foper<double, std::plus>;
  static constexpr auto& fsubd = foper<double, std::minus>;
  static constexpr auto& fmuld = foper<double, std::multiplies>;
  static constexpr auto& fdivd = foper<double, std::divides>;
  static constexpr auto& fmind = foper<double, FMin>;
  static constexpr auto& fmaxd = foper<double, FMax>;

  static constexpr auto& feqd = fcondop<double, std::equal_to>;
  static constexpr auto& fltd = fcondop<double, std::less>;
  static constexpr auto& fled = fcondop<double, std::less_equal>;

  static constexpr auto& fcvtwd  = CvtFpToInt<double, int32_t>;
  static constexpr auto& fcvtwud = CvtFpToInt<double, uint32_t>;

  static bool fsqrtd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, std::sqrt(R->GetFP<double>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fsgnjd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, std::copysign(R->GetFP<double>(Inst.rs1), R->GetFP<double>(Inst.rs2)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fsgnjnd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, std::copysign(R->GetFP<double>(Inst.rs1), -R->GetFP<double>(Inst.rs2)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fsgnjxd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    double rs1 = R->GetFP<double>(Inst.rs1), rs2 = R->GetFP<double>(Inst.rs2);
    R->SetFP(Inst.rd, std::copysign(rs1, std::signbit(rs1) ? -rs2 : rs2));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtsd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, double{R->GetFP<float>(Inst.rs1)});
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtds(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, static_cast<float>(R->GetFP<double>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fclassd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    double fp64 = R->GetFP<double>(Inst.rs1);
    uint64_t i64;
    memcpy(&i64, &fp64, sizeof(i64));
    bool quietNaN = (i64 & uint64_t{1}<<51) != 0;
    R->SetX(Inst.rd, fclass(fp64, quietNaN));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtdw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, static_cast<double>(R->GetX<int32_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtdwu(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetFP(Inst.rd, static_cast<double>(R->GetX<uint32_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32D Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev32DInstDefaults : RevInstDefaults {
    static constexpr RevRegClass rdClass  = RevRegClass::RegFLOAT;
    static constexpr RevRegClass rs1Class = RevRegClass::RegFLOAT;
    static constexpr RevRegClass rs2Class = RevRegClass::RegFLOAT;
  };
  std::vector<RevInstEntry> RV32DTable = {
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fld %rd, $imm(%rs1)"           ).SetOpcode( 0b0000111).SetFunct3(0b011 ).SetFunct7(0b0000000        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegGPR  ).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(    RevRegClass::RegUNKNOWN).SetFormat(RVTypeI).SetImplFunc(&fld ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsd %rs2, $imm(%rs1)"          ).SetOpcode( 0b0100111).SetFunct3(0b011 ).SetFunct7(0b0000000        ).SetrdClass(RevRegClass::RegIMM   ).Setrs1Class(RevRegClass::RegFLOAT  ).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeS).SetImplFunc(&fsd ).InstEntry},

    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmadd.d %rd, %rs1, %rs2, %rs3" ).SetOpcode( 0b1000011).SetFunct3(0b0   ).SetFunct7(0b00         ).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fmaddd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmsub.d %rd, %rs1, %rs2, %rs3" ).SetOpcode( 0b1000111).SetFunct3(0b0   ).SetFunct7(0b00         ).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fmsubd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fnmsub.d %rd, %rs1, %rs2, %rs3").SetOpcode( 0b1001011).SetFunct3(0b0   ).SetFunct7(0b00         ).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fnmsubd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fnmadd.d %rd, %rs1, %rs2, %rs3").SetOpcode( 0b1001111).SetFunct3(0b0   ).SetFunct7(0b00         ).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fnmaddd ).InstEntry},

    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fadd.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0000001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&faddd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsub.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0000101        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsubd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmul.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0001001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fmuld ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fdiv.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0001101        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fdivd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsqrt.d %rd, %rs1"             ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0101101        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsqrtd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmin.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b000 ).SetFunct7(0b0010101        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fmind ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmax.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b0010101        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fmaxd ).InstEntry},

    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnj.d %rd, %rs1, %rs2"       ).SetOpcode( 0b1010011).SetFunct3(0b000 ).SetFunct7(0b0010001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsgnjd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnjn.d %rd, %rs1, %rs2"      ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b0010001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsgnjnd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnjx.d %rd, %rs1, %rs2"      ).SetOpcode( 0b1010011).SetFunct3(0b010 ).SetFunct7(0b0010001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsgnjxd ).InstEntry},

    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.s.d %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0100000        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetfpcvtOp(0b01).SetImplFunc(&fcvtsd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.s %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0100001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetfpcvtOp(0b00).SetImplFunc(&fcvtsd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("feq.d %rd, %rs1, %rs2"         ).SetOpcode( 0b1010011).SetFunct3(0b010 ).SetFunct7(0b1010001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&feqd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("flt.d %rd, %rs1, %rs2"         ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b1010001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fltd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fle.d %rd, %rs1, %rs2"         ).SetOpcode( 0b1010011).SetFunct3(0b000 ).SetFunct7(0b1010001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegFLOAT).Setrs3Class(  RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fled ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fclass.d %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b1110001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fclassd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.w.d %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1100001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetfpcvtOp(0b00).SetImplFunc(&fcvtwd ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.wu.d %rd, %rs1"           ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1100001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetfpcvtOp(0b01).SetImplFunc(&fcvtwud ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.w %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1101001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetfpcvtOp(0b00).SetImplFunc(&fcvtdw ).InstEntry},
    {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.wu %rd, %rs1"           ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1101001        ).SetrdClass(RevRegClass::RegFLOAT      ).Setrs1Class(RevRegClass::RegFLOAT).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeR).SetfpcvtOp(0b01).SetImplFunc(&fcvtdwu ).InstEntry},
  };

  std::vector<RevInstEntry> RV32DCTable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fldsp %rd, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b001).SetrdClass(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&cfldsp).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fsdsp %rs1, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b101).Setrs2Class(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCSS).SetImplFunc(&cfsdsp).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fld %rd, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b001).Setrs1Class(RevRegClass::RegGPR).SetrdClass(RevRegClass::RegFLOAT).Setimm(FVal).SetFormat(RVCTypeCL).SetImplFunc(&cfld).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fsd %rs2, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b101).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCS).SetImplFunc(&cfsd).SetCompressed(true).InstEntry},
  };

public:
  /// RV32D: standard constructor
  RV32D( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV32D", Feature, RevMem, Output) {
    SetTable(std::move(RV32DTable));
    SetCTable(std::move(RV32DCTable));
  }
}; // end class RV32I

} // namespace SST::RevCPU

#endif
