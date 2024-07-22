//
// _Zfh_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZFH_H_
#define _SST_REVCPU_ZFH_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zfh : public RevExt {
  // Standard instructions
  static constexpr auto& flh     = fload<float16>;
  static constexpr auto& fsh     = fstore<float16>;

  // FMA instructions
  static constexpr auto& fmaddh  = fmadd<float16>;
  static constexpr auto& fmsubh  = fmsub<float16>;
  static constexpr auto& fnmsubh = fnmsub<float16>;
  static constexpr auto& fnmaddh = fnmadd<float16>;

  // Binary FP instructions
  static constexpr auto& faddh   = foper<float16, std::plus>;
  static constexpr auto& fsubh   = foper<float16, std::minus>;
  static constexpr auto& fmulh   = foper<float16, std::multiplies>;
  static constexpr auto& fdivh   = foper<float16, std::divides>;
  static constexpr auto& fminh   = foper<float16, FMin>;
  static constexpr auto& fmaxh   = foper<float16, FMax>;

  // FP Comparison instructions
  static constexpr auto& feqh    = fcondop<float16, std::equal_to>;
  static constexpr auto& flth    = fcondop<float16, std::less>;
  static constexpr auto& fleh    = fcondop<float16, std::less_equal>;

  // FP to Integer Conversion instructions
  static constexpr auto& fcvtwh  = fcvtif<int32_t, float16>;
  static constexpr auto& fcvtwuh = fcvtif<uint32_t, float16>;

  static constexpr auto& fsqrth  = fsqrt<float16>;

  static constexpr auto& fsgnjh  = fsgnj<float16>;
  static constexpr auto& fsgnjnh = fsgnjn<float16>;
  static constexpr auto& fsgnjxh = fsgnjx<float16>;

  static constexpr auto& fmvxw   = fmvif<float16>;
  static constexpr auto& fmvwx   = fmvfi<float16>;

  static constexpr auto& fclassh = fclassify<float16>;

  static constexpr auto& fcvtsw  = ;
  static constexpr auto& fcvtswu = ;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32F Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZfhInstDefaults : RevInstDefaults {
    RevZfhInstDefaults() {
      SetOpcode( 0b1010011 );
      SetrdClass( RevRegClass::RegFLOAT );
      Setrs1Class( RevRegClass::RegFLOAT );
      Setrs2Class( RevRegClass::RegFLOAT );
      SetRaiseFPE( true );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> ZfhTable = {
    RevZfhInstDefaults().SetMnemonic("flh %rd, $imm(%rs1)"            ).SetFunct3( 0b001 ).SetFunct2or7( 0b0000000 ).SetImplFunc(flh     ).SetrdClass(RevRegClass::RegFLOAT ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeI).SetOpcode( 0b0000111).SetRaiseFPE(false),
    RevZfhInstDefaults().SetMnemonic("fsh %rs2, $imm(%rs1)"           ).SetFunct3( 0b001 ).SetFunct2or7( 0b0000000 ).SetImplFunc(fsh     ).SetrdClass(RevRegClass::RegIMM   ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegFLOAT  ).SetFormat(RVTypeS).SetOpcode( 0b0100111).SetRaiseFPE(false),

    RevZfhInstDefaults().SetMnemonic("fmadd.h %rd, %rs1, %rs2, %rs3"  ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000010 ).SetImplFunc(fmaddh  ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1000011),
    RevZfhInstDefaults().SetMnemonic("fmsub.h %rd, %rs1, %rs2, %rs3"  ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000010 ).SetImplFunc(fmsubh  ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1000111),
    RevZfhInstDefaults().SetMnemonic("fnmsub.h %rd, %rs1, %rs2, %rs3" ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000010 ).SetImplFunc(fnmsubh ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1001011),
    RevZfhInstDefaults().SetMnemonic("fnmadd.h %rd, %rs1, %rs2, %rs3" ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000010 ).SetImplFunc(fnmaddh ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1001111),

    RevZfhInstDefaults().SetMnemonic("fadd.h %rd, %rs1, %rs2"         ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000010 ).SetImplFunc(faddh   ),
    RevZfhInstDefaults().SetMnemonic("fsub.h %rd, %rs1, %rs2"         ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000110 ).SetImplFunc(fsubh   ),
    RevZfhInstDefaults().SetMnemonic("fmul.h %rd, %rs1, %rs2"         ).SetFunct3( 0b000 ).SetFunct2or7( 0b0001010 ).SetImplFunc(fmulh   ),
    RevZfhInstDefaults().SetMnemonic("fdiv.h %rd, %rs1, %rs2"         ).SetFunct3( 0b000 ).SetFunct2or7( 0b0001110 ).SetImplFunc(fdivh   ),
    RevZfhInstDefaults().SetMnemonic("fsqrt.h %rd, %rs1"              ).SetFunct3( 0b000 ).SetFunct2or7( 0b0101110 ).SetImplFunc(fsqrth  ).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("fmin.h %rd, %rs1, %rs2"         ).SetFunct3( 0b000 ).SetFunct2or7( 0b0010110 ).SetImplFunc(fminh   ),
    RevZfhInstDefaults().SetMnemonic("fmax.h %rd, %rs1, %rs2"         ).SetFunct3( 0b001 ).SetFunct2or7( 0b0010110 ).SetImplFunc(fmaxh   ),
    RevZfhInstDefaults().SetMnemonic("fsgnj.h %rd, %rs1, %rs2"        ).SetFunct3( 0b000 ).SetFunct2or7( 0b0010010 ).SetImplFunc(fsgnjh  ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ),
    RevZfhInstDefaults().SetMnemonic("fmv.h %rd, %rs"                 ).SetFunct3( 0b000 ).SetFunct2or7( 0b0010010 ).SetImplFunc(fsgnjh  ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ),
    RevZfhInstDefaults().SetMnemonic("fsgnjn.h %rd, %rs1, %rs2"       ).SetFunct3( 0b001 ).SetFunct2or7( 0b0010010 ).SetImplFunc(fsgnjnh ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ),
    RevZfhInstDefaults().SetMnemonic("fneg.h %rd, %rs"                ).SetFunct3( 0b001 ).SetFunct2or7( 0b0010010 ).SetImplFunc(fsgnjnh ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ),
    RevZfhInstDefaults().SetMnemonic("fsgnjx.h %rd, %rs1, %rs2"       ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010010 ).SetImplFunc(fsgnjxh ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ),
    RevZfhInstDefaults().SetMnemonic("fabs.h %rd, %rs"                ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010010 ).SetImplFunc(fsgnjxh ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ),
    RevZfhInstDefaults().SetMnemonic("fcvt.w.h %rd, %rs1"             ).SetFunct3( 0b000 ).SetFunct2or7( 0b1100010 ).SetImplFunc(fcvtwh  ).SetrdClass (RevRegClass::RegGPR).Setrs2fcvtOp(0b00).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("fcvt.wu.h %rd, %rs1"            ).SetFunct3( 0b000 ).SetFunct2or7( 0b1100010 ).SetImplFunc(fcvtwuh ).SetrdClass (RevRegClass::RegGPR).Setrs2fcvtOp(0b01).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("fmv.x.w %rd, %rs1"              ).SetFunct3( 0b000 ).SetFunct2or7( 0b1110010 ).SetImplFunc(fmvxw   ).SetrdClass (RevRegClass::RegGPR).SetRaiseFPE(false).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("feq.h %rd, %rs1, %rs2"          ).SetFunct3( 0b010 ).SetFunct2or7( 0b1010010 ).SetImplFunc(feqh    ).SetrdClass (RevRegClass::RegGPR),
    RevZfhInstDefaults().SetMnemonic("flt.h %rd, %rs1, %rs2"          ).SetFunct3( 0b001 ).SetFunct2or7( 0b1010010 ).SetImplFunc(flth    ).SetrdClass (RevRegClass::RegGPR),
    RevZfhInstDefaults().SetMnemonic("fle.h %rd, %rs1, %rs2"          ).SetFunct3( 0b000 ).SetFunct2or7( 0b1010010 ).SetImplFunc(fleh    ).SetrdClass (RevRegClass::RegGPR),
    RevZfhInstDefaults().SetMnemonic("fclass.h %rd, %rs1"             ).SetFunct3( 0b001 ).SetFunct2or7( 0b1110010 ).SetImplFunc(fclasss ).SetrdClass (RevRegClass::RegGPR).SetRaiseFPE(false).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("fcvt.s.w %rd, %rs1"             ).SetFunct3( 0b000 ).SetFunct2or7( 0b1101010 ).SetImplFunc(fcvtsw  ).Setrs1Class(RevRegClass::RegGPR).Setrs2fcvtOp(0b00).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("fcvt.s.wu %rd, %rs1"            ).SetFunct3( 0b000 ).SetFunct2or7( 0b1101010 ).SetImplFunc(fcvtswu ).Setrs1Class(RevRegClass::RegGPR).Setrs2fcvtOp(0b01).Setrs2Class(RevRegClass::RegUNKNOWN),
    RevZfhInstDefaults().SetMnemonic("fmv.w.x %rd, %rs1"              ).SetFunct3( 0b000 ).SetFunct2or7( 0b1111010 ).SetImplFunc(fmvwx   ).Setrs1Class(RevRegClass::RegGPR).SetRaiseFPE(false).Setrs2Class(RevRegClass::RegUNKNOWN),
  };

  static std::vector<RevInstEntry> ZfhCOTable() {
    return {
      RevCInstDefaults().SetMnemonic("c.flwsp %rd, $imm"     ).SetFunct3(0b011).SetImplFunc(cflwsp).Setimm(FVal).SetrdClass(RevRegClass::RegFLOAT  )                                   .SetFormat(RVCTypeCI ).SetOpcode(0b10),
      RevCInstDefaults().SetMnemonic("c.fswsp %rs2, $imm"    ).SetFunct3(0b111).SetImplFunc(cfswsp).Setimm(FVal).SetrdClass(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegFLOAT).SetFormat(RVCTypeCSS).SetOpcode(0b10),
      RevCInstDefaults().SetMnemonic("c.flw %rd, %rs1, $imm" ).SetFunct3(0b011).SetImplFunc(cflw  ).Setimm(FVal).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegGPR)  .SetFormat(RVCTypeCL ).SetOpcode(0b00),
      RevCInstDefaults().SetMnemonic("c.fsw %rs2, %rs1, $imm").SetFunct3(0b111).SetImplFunc(cfsw  ).Setimm(FVal).SetrdClass(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegFLOAT).SetFormat(RVCTypeCS ).SetOpcode(0b00),
    };
  }

  // clang-format on

public:
  /// Zfh: standard constructor
  Zfh( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zfh", Feature, RevMem, Output ) {
    SetTable( std::move( ZfhTable ) );
  }
};  // end class Zfh

}  // namespace SST::RevCPU

#endif
