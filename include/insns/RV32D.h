//
// _RV32D_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32D_H_
#define _SST_REVCPU_RV32D_H_

#include "./RevExt.h"
#include "./RevInstHelpers.h"

#include <cmath>
#include <vector>

namespace SST::RevCPU {

class RV32D : public RevExt {
  // Standard instructions
  static constexpr auto& fld     = fload<double>;
  static constexpr auto& fsd     = fstore<double>;

  // FMA instructions
  static constexpr auto& fmaddd  = fmadd<double>;
  static constexpr auto& fmsubd  = fmsub<double>;
  static constexpr auto& fnmsubd = fnmsub<double>;
  static constexpr auto& fnmaddd = fnmadd<double>;

  // Binary FP instructions
  static constexpr auto& faddd   = foper<double, std::plus>;
  static constexpr auto& fsubd   = foper<double, std::minus>;
  static constexpr auto& fmuld   = foper<double, std::multiplies>;
  static constexpr auto& fdivd   = foper<double, std::divides>;
  static constexpr auto& fmind   = foper<double, FMin>;
  static constexpr auto& fmaxd   = foper<double, FMax>;

  // FP Comparison instructions
  static constexpr auto& feqd    = fcondop<double, std::equal_to>;
  static constexpr auto& fltd    = fcondop<double, std::less>;
  static constexpr auto& fled    = fcondop<double, std::less_equal>;

  // FP to Integer Conversion instructions
  static constexpr auto& fcvtwd  = CvtFpToInt<double, int32_t>;
  static constexpr auto& fcvtwud = CvtFpToInt<double, uint32_t>;

  static bool fsqrtd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, std::sqrt( R->GetFP<double>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fsgnjd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, std::copysign( R->GetFP<double>( Inst.rs1 ), R->GetFP<double>( Inst.rs2 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fsgnjnd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, std::copysign( R->GetFP<double>( Inst.rs1 ), -R->GetFP<double>( Inst.rs2 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fsgnjxd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    double rs1 = R->GetFP<double>( Inst.rs1 ), rs2 = R->GetFP<double>( Inst.rs2 );
    R->SetFP( Inst.rd, std::copysign( rs1, std::signbit( rs1 ) ? -rs2 : rs2 ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fcvtsd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, static_cast<float>( R->GetFP<double>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fcvtds( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, static_cast<double>( R->GetFP<float>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fclassd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    double   fp64 = R->GetFP<double>( Inst.rs1 );
    uint64_t i64;
    memcpy( &i64, &fp64, sizeof( i64 ) );
    bool quietNaN = ( i64 & uint64_t{ 1 } << 51 ) != 0;
    R->SetX( Inst.rd, fclass( fp64, quietNaN ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fcvtdw( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, static_cast<double>( R->GetX<int32_t>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fcvtdwu( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, static_cast<double>( R->GetX<uint32_t>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  // Compressed instructions
  static constexpr auto& cfldsp = fld;
  static constexpr auto& cfsdsp = fsd;
  static constexpr auto& cfld   = fld;
  static constexpr auto& cfsd   = fsd;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32D Instructions
  //
  // ----------------------------------------------------------------------
  struct Rev32DInstDefaults : RevInstDefaults {
    Rev32DInstDefaults() {
      SetOpcode( 0b1010011 );
      SetrdClass( RevRegClass::RegFLOAT );
      Setrs1Class( RevRegClass::RegFLOAT );
      Setrs2Class( RevRegClass::RegFLOAT );
      SetRaiseFPE( true );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> RV32DTable = {
    { Rev32DInstDefaults().SetMnemonic("fld %rd, $imm(%rs1)"           ).SetFunct3(0b011).SetFunct2or7(0b0000000).SetImplFunc(fld    ).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).SetFormat(RVTypeI).SetOpcode(0b0000111).SetRaiseFPE(false) },
    { Rev32DInstDefaults().SetMnemonic("fsd %rs2, $imm(%rs1)"          ).SetFunct3(0b011).SetFunct2or7(0b0000000).SetImplFunc(fsd    ).Setrs1Class(RevRegClass::RegGPR).SetrdClass (RevRegClass::RegIMM).SetFormat(RVTypeS).SetOpcode(0b0100111).SetRaiseFPE(false) },

    { Rev32DInstDefaults().SetMnemonic("fmadd.d %rd, %rs1, %rs2, %rs3" ).SetFunct3(0b000).SetFunct2or7(0b0000001).SetImplFunc(fmaddd ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode(0b1000011) },
    { Rev32DInstDefaults().SetMnemonic("fmsub.d %rd, %rs1, %rs2, %rs3" ).SetFunct3(0b000).SetFunct2or7(0b0000001).SetImplFunc(fmsubd ).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode(0b1000111) },
    { Rev32DInstDefaults().SetMnemonic("fnmsub.d %rd, %rs1, %rs2, %rs3").SetFunct3(0b000).SetFunct2or7(0b0000001).SetImplFunc(fnmsubd).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode(0b1001011) },
    { Rev32DInstDefaults().SetMnemonic("fnmadd.d %rd, %rs1, %rs2, %rs3").SetFunct3(0b000).SetFunct2or7(0b0000001).SetImplFunc(fnmaddd).Setrs3Class(RevRegClass::RegFLOAT).SetFormat(RVTypeR4).SetOpcode(0b1001111) },

    { Rev32DInstDefaults().SetMnemonic("fadd.d %rd, %rs1, %rs2"        ).SetFunct3(0b000).SetFunct2or7(0b0000001).SetImplFunc(faddd  ) },
    { Rev32DInstDefaults().SetMnemonic("fsub.d %rd, %rs1, %rs2"        ).SetFunct3(0b000).SetFunct2or7(0b0000101).SetImplFunc(fsubd  ) },
    { Rev32DInstDefaults().SetMnemonic("fmul.d %rd, %rs1, %rs2"        ).SetFunct3(0b000).SetFunct2or7(0b0001001).SetImplFunc(fmuld  ) },
    { Rev32DInstDefaults().SetMnemonic("fdiv.d %rd, %rs1, %rs2"        ).SetFunct3(0b000).SetFunct2or7(0b0001101).SetImplFunc(fdivd  ) },
    { Rev32DInstDefaults().SetMnemonic("fsqrt.d %rd, %rs1"             ).SetFunct3(0b000).SetFunct2or7(0b0101101).SetImplFunc(fsqrtd ).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev32DInstDefaults().SetMnemonic("fmin.d %rd, %rs1, %rs2"        ).SetFunct3(0b000).SetFunct2or7(0b0010101).SetImplFunc(fmind  ) },
    { Rev32DInstDefaults().SetMnemonic("fmax.d %rd, %rs1, %rs2"        ).SetFunct3(0b001).SetFunct2or7(0b0010101).SetImplFunc(fmaxd  ) },
    { Rev32DInstDefaults().SetMnemonic("fsgnj.d %rd, %rs1, %rs2"       ).SetFunct3(0b000).SetFunct2or7(0b0010001).SetImplFunc(fsgnjd ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ) },
    { Rev32DInstDefaults().SetMnemonic("fmv.d %rd, %rs"                ).SetFunct3(0b000).SetFunct2or7(0b0010001).SetImplFunc(fsgnjd ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ) },
    { Rev32DInstDefaults().SetMnemonic("fsgnjn.d %rd, %rs1, %rs2"      ).SetFunct3(0b001).SetFunct2or7(0b0010001).SetImplFunc(fsgnjnd).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ) },
    { Rev32DInstDefaults().SetMnemonic("fneg.d %rd, %rs"               ).SetFunct3(0b001).SetFunct2or7(0b0010001).SetImplFunc(fsgnjnd).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ) },
    { Rev32DInstDefaults().SetMnemonic("fsgnjx.d %rd, %rs1, %rs2"      ).SetFunct3(0b010).SetFunct2or7(0b0010001).SetImplFunc(fsgnjxd).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != DECODE_RS2( Inst ); } ) },
    { Rev32DInstDefaults().SetMnemonic("fabs.d %rd, %rs"               ).SetFunct3(0b010).SetFunct2or7(0b0010001).SetImplFunc(fsgnjxd).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == DECODE_RS2( Inst ); } ) },
    { Rev32DInstDefaults().SetMnemonic("fcvt.s.d %rd, %rs1"            ).SetFunct3(0b000).SetFunct2or7(0b0100000).SetImplFunc(fcvtsd ).Setrs2Class(RevRegClass::RegUNKNOWN).SetfpcvtOp(0b01) },
    { Rev32DInstDefaults().SetMnemonic("fcvt.d.s %rd, %rs1"            ).SetFunct3(0b000).SetFunct2or7(0b0100001).SetImplFunc(fcvtds ).Setrs2Class(RevRegClass::RegUNKNOWN).SetfpcvtOp(0b00) },
    { Rev32DInstDefaults().SetMnemonic("feq.d %rd, %rs1, %rs2"         ).SetFunct3(0b010).SetFunct2or7(0b1010001).SetImplFunc(feqd   ).SetrdClass (RevRegClass::RegGPR) },
    { Rev32DInstDefaults().SetMnemonic("flt.d %rd, %rs1, %rs2"         ).SetFunct3(0b001).SetFunct2or7(0b1010001).SetImplFunc(fltd   ).SetrdClass (RevRegClass::RegGPR) },
    { Rev32DInstDefaults().SetMnemonic("fle.d %rd, %rs1, %rs2"         ).SetFunct3(0b000).SetFunct2or7(0b1010001).SetImplFunc(fled   ).SetrdClass (RevRegClass::RegGPR) },
    { Rev32DInstDefaults().SetMnemonic("fclass.d %rd, %rs1"            ).SetFunct3(0b001).SetFunct2or7(0b1110001).SetImplFunc(fclassd).SetrdClass (RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev32DInstDefaults().SetMnemonic("fcvt.w.d %rd, %rs1"            ).SetFunct3(0b000).SetFunct2or7(0b1100001).SetImplFunc(fcvtwd ).SetrdClass (RevRegClass::RegGPR).SetfpcvtOp(0b00).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev32DInstDefaults().SetMnemonic("fcvt.wu.d %rd, %rs1"           ).SetFunct3(0b000).SetFunct2or7(0b1100001).SetImplFunc(fcvtwud).SetrdClass (RevRegClass::RegGPR).SetfpcvtOp(0b01).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev32DInstDefaults().SetMnemonic("fcvt.d.w %rd, %rs1"            ).SetFunct3(0b000).SetFunct2or7(0b1101001).SetImplFunc(fcvtdw ).Setrs1Class(RevRegClass::RegGPR).SetfpcvtOp(0b00).Setrs2Class(RevRegClass::RegUNKNOWN) },
    { Rev32DInstDefaults().SetMnemonic("fcvt.d.wu %rd, %rs1"           ).SetFunct3(0b000).SetFunct2or7(0b1101001).SetImplFunc(fcvtdwu).Setrs1Class(RevRegClass::RegGPR).SetfpcvtOp(0b01).Setrs2Class(RevRegClass::RegUNKNOWN) },
  };

  std::vector<RevInstEntry> RV32DCTable = {
    { RevCInstDefaults().SetMnemonic("c.fldsp %rd, $imm"     ).SetOpcode(0b10).SetFunct3(0b001).SetImplFunc(cfldsp).SetrdClass (RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).Setimm(FVal).SetFormat(RVCTypeCI ) },
    { RevCInstDefaults().SetMnemonic("c.fsdsp %rs1, $imm"    ).SetOpcode(0b10).SetFunct3(0b101).SetImplFunc(cfsdsp).Setrs1Class(RevRegClass::RegGPR  ).Setrs2Class(RevRegClass::RegFLOAT).Setimm(FVal).SetFormat(RVCTypeCSS) },
    { RevCInstDefaults().SetMnemonic("c.fld %rd, %rs1, $imm" ).SetOpcode(0b00).SetFunct3(0b001).SetImplFunc(cfld  ).SetrdClass (RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).Setimm(FVal).SetFormat(RVCTypeCL ) },
    { RevCInstDefaults().SetMnemonic("c.fsd %rs2, %rs1, $imm").SetOpcode(0b00).SetFunct3(0b101).SetImplFunc(cfsd  ).Setrs1Class(RevRegClass::RegGPR  ).Setrs2Class(RevRegClass::RegFLOAT).Setimm(FVal).SetFormat(RVCTypeCS ) },
  };
  // clang-format on

public:
  /// RV32D: standard constructor
  RV32D( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "RV32D", Feature, RevMem, Output ) {
    SetTable( std::move( RV32DTable ) );
    SetCTable( std::move( RV32DCTable ) );
  }
};  // end class RV32I

}  // namespace SST::RevCPU

#endif
