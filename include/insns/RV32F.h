//
// _RV32F_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32F_H_
#define _SST_REVCPU_RV32F_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>
#include <cmath>

namespace SST::RevCPU{

class RV32F : public RevExt{

  // Compressed instructions
  static bool cflwsp(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    // c.flwsp rd, $imm = lw rd, x2, $imm
    return flw(F, R, M, Inst);
  }

  static bool cfswsp(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    // c.swsp rs2, $imm = sw rs2, x2, $imm
    return fsw(F, R, M, Inst);
  }

  static bool cflw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    // c.flw %rd, %rs1, $imm = flw %rd, %rs1, $imm
    return flw(F, R, M, Inst);
  }

  static bool cfsw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    // c.fsw rs2, rs1, $imm = fsw rs2, $imm(rs1)
    return fsw(F, R, M, Inst);
  }

  // Standard instructions
  static constexpr auto& flw = fload<float>;
  static constexpr auto& fsw = fstore<float>;

  // FMA instructions
  static constexpr auto& fmadds  = fmadd<float>;
  static constexpr auto& fmsubs  = fmsub<float>;
  static constexpr auto& fnmsubs = fnmsub<float>;
  static constexpr auto& fnmadds = fnmadd<float>;

  // Binary FP instructions
  static constexpr auto& fadds = foper<float, std::plus>;
  static constexpr auto& fsubs = foper<float, std::minus>;
  static constexpr auto& fmuls = foper<float, std::multiplies>;
  static constexpr auto& fdivs = foper<float, std::divides>;
  static constexpr auto& fmins = foper<float, FMin>;
  static constexpr auto& fmaxs = foper<float, FMax>;

  // FP Comparison instructions
  static constexpr auto& feqs = fcondop<float, std::equal_to>;
  static constexpr auto& flts = fcondop<float, std::less>;
  static constexpr auto& fles = fcondop<float, std::less_equal>;

  // FP to Integer Conversion instructions
  static constexpr auto& fcvtws  = CvtFpToInt<float,  int32_t>;
  static constexpr auto& fcvtwus = CvtFpToInt<float, uint32_t>;

  static bool fsqrts(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, sqrtf( R->GetFP<float>(Inst.rs1) ));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fsgnjs(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, std::copysign( R->GetFP<float>(Inst.rs1), R->GetFP<float>(Inst.rs2) ));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fsgnjns(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, std::copysign( R->GetFP<float>(Inst.rs1), -R->GetFP<float>(Inst.rs2) ));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fsgnjxs(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    float rs1 = R->GetFP<float>(Inst.rs1), rs2 = R->GetFP<float>(Inst.rs2);
    R->SetFP(Inst.rd, std::copysign(rs1, std::signbit(rs1) ? -rs2 : rs2));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmvxw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    int32_t i32;
    float fp32 = R->GetFP<float, true>(Inst.rs1); // The FP32 value
    memcpy(&i32, &fp32, sizeof(i32));       // Reinterpreted as int32_t
    R->SetX(Inst.rd, i32);                  // Copied to the destination register
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmvwx(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    float fp32;
    auto i32 = R->GetX<int32_t>(Inst.rs1);  // The X register as a 32-bit value
    memcpy(&fp32, &i32, sizeof(fp32));      // Reinterpreted as float
    R->SetFP(Inst.rd, fp32);                // Copied to the destination register
    R->AdvancePC(Inst);
    return true;
  }

  static bool fclasss(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    float fp32 = R->GetFP<float>(Inst.rs1);
    uint32_t i32;
    memcpy(&i32, &fp32, sizeof(i32));
    bool quietNaN = (i32 & uint32_t{1}<<22) != 0;
    R->SetX(Inst.rd, fclass(fp32, quietNaN));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtsw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, static_cast<float>(R->GetX<int32_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtswu(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, static_cast<float>(R->GetX<uint32_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32F Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev32FInstDefaults : RevInstDefaults {
    Rev32FInstDefaults(){
      SetFormat(RVTypeR);
      SetOpcode(0b1010011);
      SetrdClass (RevRegClass::RegFLOAT);
      Setrs1Class(RevRegClass::RegFLOAT);
      Setrs2Class(RevRegClass::RegFLOAT);
      Setrs3Class(RevRegClass::RegUNKNOWN);
      SetRaiseFPE(true);
    }
  };

  std::vector<RevInstEntry> RV32FTable = {
    { Rev32FInstDefaults().SetMnemonic("flw %rd, $imm(%rs1)"                  ).SetFunct3(0b010).SetFunct2or7(0b0000000).SetImplFunc(flw     ).SetrdClass(RevRegClass::RegFLOAT ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeI).SetOpcode( 0b0000111).SetRaiseFPE(false) },
    { Rev32FInstDefaults().SetMnemonic("fsw %rs2, $imm(%rs1)"                 ).SetFunct3(0b010).SetFunct2or7(0b0000000).SetImplFunc(fsw     ).SetrdClass(RevRegClass::RegIMM   ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegFLOAT  ).SetFormat(RVTypeS).SetOpcode( 0b0100111).SetRaiseFPE(false) },

    { Rev32FInstDefaults().SetMnemonic("fmadd.s %rd, %rs1, %rs2, %rs3"        ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fmadds  ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1000011) },
    { Rev32FInstDefaults().SetMnemonic("fmsub.s %rd, %rs1, %rs2, %rs3"        ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fmsubs  ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1000111) },
    { Rev32FInstDefaults().SetMnemonic("fnmsub.s %rd, %rs1, %rs2, %rs3"       ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fnmsubs ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1001011) },
    { Rev32FInstDefaults().SetMnemonic("fnmadd.s %rd, %rs1, %rs2, %rs3"       ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fnmadds ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1001111) },

    { Rev32FInstDefaults().SetMnemonic("fadd.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0000000).SetImplFunc(fadds   ) },
    { Rev32FInstDefaults().SetMnemonic("fsub.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0000100).SetImplFunc(fsubs   ) },
    { Rev32FInstDefaults().SetMnemonic("fmul.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0001000).SetImplFunc(fmuls   ) },
    { Rev32FInstDefaults().SetMnemonic("fdiv.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0001100).SetImplFunc(fdivs   ) },
    { Rev32FInstDefaults().SetMnemonic("fsqrt.s %rd, %rs1"                    ).SetFunct3(0b000).SetFunct2or7(0b0101100).SetImplFunc(fsqrts  ).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev32FInstDefaults().SetMnemonic("fmin.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0010100).SetImplFunc(fmins   ) },
    { Rev32FInstDefaults().SetMnemonic("fmax.s %rd, %rs1, %rs2"               ).SetFunct3(0b001).SetFunct2or7(0b0010100).SetImplFunc(fmaxs   ) },
    { Rev32FInstDefaults().SetMnemonic("fsgnj.s %rd, %rs1, %rs2"              ).SetFunct3(0b000).SetFunct2or7(0b0010000).SetImplFunc(fsgnjs  ).SetRaiseFPE(false) },
    { Rev32FInstDefaults().SetMnemonic("fsgnjn.s %rd, %rs1, %rs2"             ).SetFunct3(0b001).SetFunct2or7(0b0010000).SetImplFunc(fsgnjns ).SetRaiseFPE(false) },
    { Rev32FInstDefaults().SetMnemonic("fsgnjx.s %rd, %rs1, %rs2"             ).SetFunct3(0b010).SetFunct2or7(0b0010000).SetImplFunc(fsgnjxs ).SetRaiseFPE(false) },
    { Rev32FInstDefaults().SetMnemonic("fcvt.w.s %rd, %rs1"                   ).SetFunct3(0b000).SetFunct2or7(0b1100000).SetImplFunc(fcvtws  ).Setrs2Class(RevRegClass::RegUNKNOWN).SetfpcvtOp(0b00) },
    { Rev32FInstDefaults().SetMnemonic("fcvt.wu.s %rd, %rs1"                  ).SetFunct3(0b000).SetFunct2or7(0b1100000).SetImplFunc(fcvtwus ).Setrs2Class(RevRegClass::RegUNKNOWN).SetfpcvtOp(0b01) },
    { Rev32FInstDefaults().SetMnemonic("fmv.x.w %rd, %rs1"                    ).SetFunct3(0b000).SetFunct2or7(0b1110000).SetImplFunc(fmvxw   ).Setrs2Class(RevRegClass::RegUNKNOWN).SetRaiseFPE(false) },
    { Rev32FInstDefaults().SetMnemonic("feq.s %rd, %rs1, %rs2"                ).SetFunct3(0b010).SetFunct2or7(0b1010000).SetImplFunc(feqs    ).SetrdClass(RevRegClass::RegGPR) },
    { Rev32FInstDefaults().SetMnemonic("flt.s %rd, %rs1, %rs2"                ).SetFunct3(0b001).SetFunct2or7(0b1010000).SetImplFunc(flts    ).SetrdClass(RevRegClass::RegGPR) },
    { Rev32FInstDefaults().SetMnemonic("fle.s %rd, %rs1, %rs2"                ).SetFunct3(0b000).SetFunct2or7(0b1010000).SetImplFunc(fles    ).SetrdClass(RevRegClass::RegGPR) },
    { Rev32FInstDefaults().SetMnemonic("fclass.s %rd, %rs1"                   ).SetFunct3(0b001).SetFunct2or7(0b1110000).SetImplFunc(fclasss ).Setrs2Class(RevRegClass::RegUNKNOWN).SetRaiseFPE(false) },
    { Rev32FInstDefaults().SetMnemonic("fcvt.s.w %rd, %rs1"                   ).SetFunct3(0b000).SetFunct2or7(0b1101000).SetImplFunc(fcvtsw  ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).SetfpcvtOp(0b00) },
    { Rev32FInstDefaults().SetMnemonic("fcvt.s.wu %rd, %rs1"                  ).SetFunct3(0b000).SetFunct2or7(0b1101000).SetImplFunc(fcvtswu ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).SetfpcvtOp(0b01) },
    { Rev32FInstDefaults().SetMnemonic("fmv.w.x %rd, %rs1"                    ).SetFunct3(0b000).SetFunct2or7(0b1111000).SetImplFunc(fmvwx   ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).SetRaiseFPE(false) },
  };

  std::vector<RevInstEntry> RV32FCOTable = {
    { RevCInstDefaults().SetMnemonic("c.flwsp %rd, $imm"     ).SetOpcode(0b10).SetFunct3(0b011).SetImplFunc(cflwsp).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI) },
    { RevCInstDefaults().SetMnemonic("c.fswsp %rs2, $imm"    ).SetOpcode(0b10).SetFunct3(0b111).SetImplFunc(cfswsp).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegFLOAT).Setimm(FVal).SetFormat(RVCTypeCSS) },
    { RevCInstDefaults().SetMnemonic("c.flw %rd, %rs1, $imm" ).SetOpcode(0b00).SetFunct3(0b011).SetImplFunc(cflw  ).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCL) },
    { RevCInstDefaults().SetMnemonic("c.fsw %rs2, %rs1, $imm").SetOpcode(0b00).SetFunct3(0b111).SetImplFunc(cfsw  ).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegFLOAT).Setimm(FVal).SetFormat(RVCTypeCS) },
  };

public:
  /// RV32F: standard constructor
  RV32F( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV32F", Feature, RevMem, Output) {
    SetTable(std::move(RV32FTable));
    SetOTable(std::move(RV32FCOTable));
  }
}; // end class RV32F

} // namespace SST::RevCPU

#endif
