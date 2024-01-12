//
// _RV64I_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64I_H_
#define _SST_REVCPU_RV64I_H_

#include "../RevInstHelpers.h"

#include <vector>

namespace SST::RevCPU{

class RV64I : public RevExt{

  // Compressed instructions
  static bool cldsp(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.ldsp rd, $imm = lw rd, x2, $imm
    // Inst.rs1  = 2;  //Removed - set in decode
    //ZEXT(Inst.imm, ((Inst.imm&0b111111))*8, 32);
    //Inst.imm = ((Inst.imm & 0b111111)*8);
    Inst.imm = ((Inst.imm & 0b111111111));  //Bits placed correctly in decode, no need to scale
    return ld(F, R, M, Inst);
  }

  static bool csdsp(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.swsp rs2, $imm = sw rs2, x2, $imm
    // Inst.rs1  = 2; //Removed - set in decode
    //ZEXT(Inst.imm, ((Inst.imm&0b111111))*8, 32);
    //Inst.imm = ((Inst.imm & 0b111111)*8);
    Inst.imm = ((Inst.imm & 0b1111111111)); // bits placed correctly in decode, no need to scale
    return sd(F, R, M, Inst);
  }

  static bool cld(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.ld %rd, %rs1, $imm = ld %rd, %rs1, $imm
    //Inst.rd  = CRegIdx(Inst.rd);  //Removed - scaled in decode
    //Inst.rs1 = CRegIdx(Inst.rs1); //Removed - scaled in decode
    //Inst.imm = ((Inst.imm&0b11111)*8);
    Inst.imm = (Inst.imm&0b11111111); //8-bit immd, zero-extended, scaled at decode
    return ld(F, R, M, Inst);
  }

  static bool csd(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.sd rs2, rs1, $imm = sd rs2, $imm(rs1)
    //Inst.rs2 = CRegIdx(Inst.rs2);  //Removed - scaled in decode
    //Inst.rs1 = CRegIdx(Inst.rs1); // Removed  - scaled in decode
    Inst.imm = (Inst.imm&0b11111111); //imm is 8-bits, zero extended, decoder pre-aligns bits, no scaling needed
    return sd(F, R, M, Inst);
  }

  static bool caddiw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.addiw %rd, $imm = addiw %rd, %rd, $imm
    // Inst.rs1 = Inst.rd; //Removed - set in decode
    // uint64_t tmp = Inst.imm & 0b111111;
    // SEXT(Inst.imm, tmp, 6);
    Inst.imm = Inst.ImmSignExt(6);
    return addiw(F, R, M, Inst);
  }

  static bool caddw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.addw %rd, %rs2 = addw %rd, %rd, %rs2
    //Inst.rd  = CRegIdx(Inst.rd);  //Removed - set in decode
    //Inst.rs1 = Inst.rd;  //Removed - set in decode
    //Inst.rs2  = CRegIdx(Inst.rs2);  //Removed - set in decode
    return addw(F, R, M, Inst);
  }

  static bool csubw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.subw %rd, %rs2 = subw %rd, %rd, %rs2
    //Inst.rd  = CRegIdx(Inst.rd);  //Removed - set in decode
    //Inst.rs1 = Inst.rd;  //Removed - set in decode
    //Inst.rs2  = CRegIdx(Inst.rs2);  //Removed - set in decode
    return subw(F, R, M, Inst);
  }

  // Standard instructions
  static constexpr auto& ld    = load<int64_t>;
  static constexpr auto& lwu   = load<uint32_t>;
  static constexpr auto& sd    = store<uint64_t>;

  // 32-bit arithmetic operators
  static constexpr auto& addw  = oper<std::plus,   OpKind::Reg, std::make_signed_t,   true>;
  static constexpr auto& subw  = oper<std::minus,  OpKind::Reg, std::make_signed_t,   true>;
  static constexpr auto& addiw = oper<std::plus,   OpKind::Imm, std::make_signed_t,   true>;

  // Shift operators
  static constexpr auto& slliw = oper<ShiftLeft,  OpKind::Imm, std::make_unsigned_t, true>;
  static constexpr auto& srliw = oper<ShiftRight, OpKind::Imm, std::make_unsigned_t, true>;
  static constexpr auto& sraiw = oper<ShiftRight, OpKind::Imm, std::make_signed_t,   true>;
  static constexpr auto& sllw  = oper<ShiftLeft,  OpKind::Reg, std::make_unsigned_t, true>;
  static constexpr auto& srlw  = oper<ShiftRight, OpKind::Reg, std::make_unsigned_t, true>;
  static constexpr auto& sraw  = oper<ShiftRight, OpKind::Reg, std::make_signed_t,   true>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64I Instructions
  //
  // ----------------------------------------------------------------------

  struct Rev64InstDefaults : RevInstDefaults {
    Rev64InstDefaults(){
      SetOpcode(0b0111011);
    }
  };

  std::vector<RevInstEntry> RV64ITable = {
    { Rev64InstDefaults().SetMnemonic("lwu %rd, $imm(%rs1)"   ).SetFunct3(0b110).SetImplFunc(lwu   ).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0000011).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev64InstDefaults().SetMnemonic("ld %rd, $imm(%rs1)"    ).SetFunct3(0b011).SetImplFunc(ld    ).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0000011).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev64InstDefaults().SetMnemonic("sd %rs2, $imm(%rs1)"   ).SetFunct3(0b011).SetImplFunc(sd    ).             SetFormat(RVTypeS).SetOpcode(0b0100011).SetrdClass (RevRegClass::RegIMM    ) },
    { Rev64InstDefaults().SetMnemonic("addiw %rd, %rs1, $imm" ).SetFunct3(0b000).SetImplFunc(addiw ).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0011011).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev64InstDefaults().SetMnemonic("slliw %rd, %rs1, $imm" ).SetFunct3(0b001).SetImplFunc(slliw ).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0011011).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev64InstDefaults().SetMnemonic("srliw %rd, %rs1, $imm" ).SetFunct3(0b101).SetImplFunc(srliw ).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0011011).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev64InstDefaults().SetMnemonic("sraiw %rd, %rs1, $imm" ).SetFunct3(0b101).SetImplFunc(sraiw ).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0011011).Setrs2Class(RevRegClass::RegUNKNOWN).SetFunct2or7(0b0100000) },
    { Rev64InstDefaults().SetMnemonic("addw %rd, %rs1, %rs2"  ).SetFunct3(0b000).SetImplFunc(addw  ) },
    { Rev64InstDefaults().SetMnemonic("subw %rd, %rs1, %rs2"  ).SetFunct3(0b000).SetImplFunc(subw  ).SetFunct2or7(0b0100000) },
    { Rev64InstDefaults().SetMnemonic("sllw %rd, %rs1, %rs2"  ).SetFunct3(0b001).SetImplFunc(sllw  ) },
    { Rev64InstDefaults().SetMnemonic("srlw %rd, %rs1, %rs2"  ).SetFunct3(0b101).SetImplFunc(srlw  ) },
    { Rev64InstDefaults().SetMnemonic("sraw %rd, %rs1, %rs2"  ).SetFunct3(0b101).SetImplFunc(sraw  ).SetFunct2or7(0b0100000) },
  };

  std::vector<RevInstEntry> RV64ICTable = {
    { RevCInstDefaults().SetMnemonic("c.ldsp %rd, $imm  "     ).SetFunct3(0b011).SetImplFunc(cldsp ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b10) },
    { RevCInstDefaults().SetMnemonic("c.sdsp %rs2, $imm"      ).SetFunct3(0b111).SetImplFunc(csdsp ).Setimm(FVal).SetFormat(RVCTypeCSS).SetOpcode(0b10) },
    { RevCInstDefaults().SetMnemonic("c.ld %rd, %rs1, $imm"   ).SetFunct3(0b011).SetImplFunc(cld   ).Setimm(FVal).SetFormat(RVCTypeCL ).SetOpcode(0b00) },
    { RevCInstDefaults().SetMnemonic("c.sd %rs2, %rs1, $imm"  ).SetFunct3(0b111).SetImplFunc(csd   ).Setimm(FVal).SetFormat(RVCTypeCS ).SetOpcode(0b00) },
    { RevCInstDefaults().SetMnemonic("c.addiw %rd, $imm"      ).SetFunct3(0b001).SetImplFunc(caddiw).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b01) },
    { RevCInstDefaults().SetMnemonic("c.addw %rd, %rs1"       ).SetFunct2( 0b01).SetImplFunc(caddw )             .SetFormat(RVCTypeCA ).SetOpcode(0b01).SetFunct6(0b100111) },
    { RevCInstDefaults().SetMnemonic("c.subw %rd, %rs1"       ).SetFunct2( 0b00).SetImplFunc(csubw )             .SetFormat(RVCTypeCA ).SetOpcode(0b01).SetFunct6(0b100111) },
  };

public:
  /// RV64I: standard constructor
  RV64I( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV64I", Feature, RevMem, Output ) {
    SetTable(std::move(RV64ITable));
    SetCTable(std::move(RV64ICTable));
  }
}; // end class RV64I
} // namespace SST::RevCPU

#endif
