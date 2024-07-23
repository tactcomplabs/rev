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

#include "../RevExt.h"
#include "../RevInstHelpers.h"

#include <cmath>
#include <vector>

namespace SST::RevCPU {

class RV32F : public RevExt {
  // Standard instructions
  static constexpr auto& flw     = fload<float>;
  static constexpr auto& fsw     = fstore<float>;

  // FMA instructions
  static constexpr auto& fmadds  = fmadd<float>;
  static constexpr auto& fmsubs  = fmsub<float>;
  static constexpr auto& fnmsubs = fnmsub<float>;
  static constexpr auto& fnmadds = fnmadd<float>;

  // Binary FP instructions
  static constexpr auto& fadds   = foper<float, std::plus>;
  static constexpr auto& fsubs   = foper<float, std::minus>;
  static constexpr auto& fmuls   = foper<float, std::multiplies>;
  static constexpr auto& fdivs   = foper<float, std::divides>;
  static constexpr auto& fmins   = foper<float, FMin>;
  static constexpr auto& fmaxs   = foper<float, FMax>;

  // FP Comparison instructions
  static constexpr auto& feqs    = fcondop<float, std::equal_to>;
  static constexpr auto& flts    = fcondop<float, std::less>;
  static constexpr auto& fles    = fcondop<float, std::less_equal>;

  // FP to Integer Conversion instructions
  static constexpr auto& fcvtws  = fcvtif<int32_t, float>;
  static constexpr auto& fcvtwus = fcvtif<uint32_t, float>;

  // Square root
  static constexpr auto& fsqrts  = fsqrt<float>;

  // Sign transfer
  static constexpr auto& fsgnjs  = fsgnj<float>;
  static constexpr auto& fsgnjns = fsgnjn<float>;
  static constexpr auto& fsgnjxs = fsgnjx<float>;

  // Transfers between integer and FP registers
  static constexpr auto& fmvxw   = fmvif<float>;
  static constexpr auto& fmvwx   = fmvfi<float>;

  // FP classification
  static constexpr auto& fclasss = fclassify<float>;

  // Conversion from integer to float
  static constexpr auto& fcvtsw  = fcvtfi<float, int32_t>;
  static constexpr auto& fcvtswu = fcvtfi<float, uint32_t>;

  // Compressed instructions
  static constexpr auto& cflwsp  = flw;
  static constexpr auto& cfswsp  = fsw;
  static constexpr auto& cflw    = flw;
  static constexpr auto& cfsw    = fsw;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32F Instructions
  //
  // ----------------------------------------------------------------------
  struct Rev32FInstDefaults : RevInstDefaults {
    Rev32FInstDefaults() {
      SetOpcode( 0b1010011 );
      SetrdClass( RevRegClass::RegFLOAT );
      Setrs1Class( RevRegClass::RegFLOAT );
      Setrs2Class( RevRegClass::RegFLOAT );
      SetRaiseFPE( true );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> RV32FTable = {
    Rev32FInstDefaults().SetMnemonic("flw %rd, $imm(%rs1)"                  ).SetFunct3(0b010).SetFunct2or7(0b0000000).SetImplFunc(flw     ).SetrdClass(RevRegClass::RegFLOAT ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeI).SetOpcode( 0b0000111).SetRaiseFPE(false),
    Rev32FInstDefaults().SetMnemonic("fsw %rs2, $imm(%rs1)"                 ).SetFunct3(0b010).SetFunct2or7(0b0000000).SetImplFunc(fsw     ).SetrdClass(RevRegClass::RegIMM   ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegFLOAT  ).SetFormat(RVTypeS).SetOpcode( 0b0100111).SetRaiseFPE(false),

    Rev32FInstDefaults().SetMnemonic("fmadd.s %rd, %rs1, %rs2, %rs3"        ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fmadds  ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1000011),
    Rev32FInstDefaults().SetMnemonic("fmsub.s %rd, %rs1, %rs2, %rs3"        ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fmsubs  ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1000111),
    Rev32FInstDefaults().SetMnemonic("fnmsub.s %rd, %rs1, %rs2, %rs3"       ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fnmsubs ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1001011),
    Rev32FInstDefaults().SetMnemonic("fnmadd.s %rd, %rs1, %rs2, %rs3"       ).SetFunct3(0b000).SetFunct2or7(0b00     ).SetImplFunc(fnmadds ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode( 0b1001111),

    Rev32FInstDefaults().SetMnemonic("fadd.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0000000).SetImplFunc(fadds   ),
    Rev32FInstDefaults().SetMnemonic("fsub.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0000100).SetImplFunc(fsubs   ),
    Rev32FInstDefaults().SetMnemonic("fmul.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0001000).SetImplFunc(fmuls   ),
    Rev32FInstDefaults().SetMnemonic("fdiv.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0001100).SetImplFunc(fdivs   ),
    Rev32FInstDefaults().SetMnemonic("fsqrt.s %rd, %rs1"                    ).SetFunct3(0b000).SetFunct2or7(0b0101100).SetImplFunc(fsqrts  ).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("fmin.s %rd, %rs1, %rs2"               ).SetFunct3(0b000).SetFunct2or7(0b0010100).SetImplFunc(fmins   ),
    Rev32FInstDefaults().SetMnemonic("fmax.s %rd, %rs1, %rs2"               ).SetFunct3(0b001).SetFunct2or7(0b0010100).SetImplFunc(fmaxs   ),
    Rev32FInstDefaults().SetMnemonic("fsgnj.s %rd, %rs1, %rs2"              ).SetFunct3(0b000).SetFunct2or7(0b0010000).SetImplFunc(fsgnjs  ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ),
    Rev32FInstDefaults().SetMnemonic("fmv.s %rd, %rs"                       ).SetFunct3(0b000).SetFunct2or7(0b0010000).SetImplFunc(fsgnjs  ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ),
    Rev32FInstDefaults().SetMnemonic("fsgnjn.s %rd, %rs1, %rs2"             ).SetFunct3(0b001).SetFunct2or7(0b0010000).SetImplFunc(fsgnjns ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ),
    Rev32FInstDefaults().SetMnemonic("fneg.s %rd, %rs"                      ).SetFunct3(0b001).SetFunct2or7(0b0010000).SetImplFunc(fsgnjns ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ),
    Rev32FInstDefaults().SetMnemonic("fsgnjx.s %rd, %rs1, %rs2"             ).SetFunct3(0b010).SetFunct2or7(0b0010000).SetImplFunc(fsgnjxs ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ),
    Rev32FInstDefaults().SetMnemonic("fabs.s %rd, %rs"                      ).SetFunct3(0b010).SetFunct2or7(0b0010000).SetImplFunc(fsgnjxs ).SetRaiseFPE(false).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ),
    Rev32FInstDefaults().SetMnemonic("fcvt.w.s %rd, %rs1"                   ).SetFunct3(0b000).SetFunct2or7(0b1100000).SetImplFunc(fcvtws  ).SetrdClass (RevRegClass::RegGPR).Setrs2fcvtOp(0b00).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("fcvt.wu.s %rd, %rs1"                  ).SetFunct3(0b000).SetFunct2or7(0b1100000).SetImplFunc(fcvtwus ).SetrdClass (RevRegClass::RegGPR).Setrs2fcvtOp(0b01).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("fmv.x.w %rd, %rs1"                    ).SetFunct3(0b000).SetFunct2or7(0b1110000).SetImplFunc(fmvxw   ).SetrdClass (RevRegClass::RegGPR).SetRaiseFPE(false).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("feq.s %rd, %rs1, %rs2"                ).SetFunct3(0b010).SetFunct2or7(0b1010000).SetImplFunc(feqs    ).SetrdClass (RevRegClass::RegGPR),
    Rev32FInstDefaults().SetMnemonic("flt.s %rd, %rs1, %rs2"                ).SetFunct3(0b001).SetFunct2or7(0b1010000).SetImplFunc(flts    ).SetrdClass (RevRegClass::RegGPR),
    Rev32FInstDefaults().SetMnemonic("fle.s %rd, %rs1, %rs2"                ).SetFunct3(0b000).SetFunct2or7(0b1010000).SetImplFunc(fles    ).SetrdClass (RevRegClass::RegGPR),
    Rev32FInstDefaults().SetMnemonic("fclass.s %rd, %rs1"                   ).SetFunct3(0b001).SetFunct2or7(0b1110000).SetImplFunc(fclasss ).SetrdClass (RevRegClass::RegGPR).SetRaiseFPE(false).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("fcvt.s.w %rd, %rs1"                   ).SetFunct3(0b000).SetFunct2or7(0b1101000).SetImplFunc(fcvtsw  ).Setrs1Class(RevRegClass::RegGPR).Setrs2fcvtOp(0b00).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("fcvt.s.wu %rd, %rs1"                  ).SetFunct3(0b000).SetFunct2or7(0b1101000).SetImplFunc(fcvtswu ).Setrs1Class(RevRegClass::RegGPR).Setrs2fcvtOp(0b01).Setrs2Class(RevRegClass::RegUNKNOWN),
    Rev32FInstDefaults().SetMnemonic("fmv.w.x %rd, %rs1"                    ).SetFunct3(0b000).SetFunct2or7(0b1111000).SetImplFunc(fmvwx   ).Setrs1Class(RevRegClass::RegGPR).SetRaiseFPE(false).Setrs2Class(RevRegClass::RegUNKNOWN),
  };

  static std::vector<RevInstEntry> RV32FCOTable() {
    return {
      RevCInstDefaults().SetMnemonic("c.flwsp %rd, $imm"     ).SetFunct3(0b011).SetImplFunc(cflwsp).Setimm(FVal).SetrdClass(RevRegClass::RegFLOAT  )                                   .SetFormat(RVCTypeCI ).SetOpcode(0b10),
      RevCInstDefaults().SetMnemonic("c.fswsp %rs2, $imm"    ).SetFunct3(0b111).SetImplFunc(cfswsp).Setimm(FVal).SetrdClass(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegFLOAT).SetFormat(RVCTypeCSS).SetOpcode(0b10),
      RevCInstDefaults().SetMnemonic("c.flw %rd, %rs1, $imm" ).SetFunct3(0b011).SetImplFunc(cflw  ).Setimm(FVal).SetrdClass(RevRegClass::RegFLOAT  ).Setrs1Class(RevRegClass::RegGPR)  .SetFormat(RVCTypeCL ).SetOpcode(0b00),
      RevCInstDefaults().SetMnemonic("c.fsw %rs2, %rs1, $imm").SetFunct3(0b111).SetImplFunc(cfsw  ).Setimm(FVal).SetrdClass(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegFLOAT).SetFormat(RVCTypeCS ).SetOpcode(0b00),
    };
  }

  // clang-format on

public:
  /// RV32F: standard constructor
  RV32F( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "RV32F", Feature, RevMem, Output ) {
    SetTable( std::move( RV32FTable ) );
    if( !Feature->IsRV64() && !Feature->HasD() ) {
      // RV32FC-only instructions
      SetCTable( RV32FCOTable() );
    }
  }
};  // end class RV32F

}  // namespace SST::RevCPU

#endif
