//
// _RV64F_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64F_H_
#define _SST_REVCPU_RV64F_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

#include <limits>
#include <vector>

namespace SST::RevCPU {

class RV64F : public RevExt {
  static constexpr auto& fcvtls  = CvtFpToInt<float, int64_t>;
  static constexpr auto& fcvtlus = CvtFpToInt<float, uint64_t>;

  static bool fcvtsl( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, static_cast<float>( R->GetX<int64_t>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool fcvtslu( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, static_cast<float>( R->GetX<uint64_t>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64F Instructions
  //
  // ----------------------------------------------------------------------
  struct Rev64FInstDefaults : RevInstDefaults {
    Rev64FInstDefaults() {
      SetOpcode( 0b1010011 );
      Setrs2Class( RevRegClass::RegUNKNOWN );
      SetRaiseFPE( true );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> RV64FTable = {
    { Rev64FInstDefaults().SetMnemonic("fcvt.l.s  %rd, %rs1").SetFunct2or7(0b1100000).SetImplFunc(fcvtls ).SetrdClass(RevRegClass::RegGPR  ).Setrs1Class(RevRegClass::RegFLOAT).SetfpcvtOp(0b00010) },
    { Rev64FInstDefaults().SetMnemonic("fcvt.lu.s %rd, %rs1").SetFunct2or7(0b1100000).SetImplFunc(fcvtlus).SetrdClass(RevRegClass::RegGPR  ).Setrs1Class(RevRegClass::RegFLOAT).SetfpcvtOp(0b00011) },
    { Rev64FInstDefaults().SetMnemonic("fcvt.s.l  %rd, %rs1").SetFunct2or7(0b1101000).SetImplFunc(fcvtsl ).SetrdClass(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).SetfpcvtOp(0b00010) },
    { Rev64FInstDefaults().SetMnemonic("fcvt.s.lu %rd, %rs1").SetFunct2or7(0b1101000).SetImplFunc(fcvtslu).SetrdClass(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).SetfpcvtOp(0b00011) },
  };
  // clang-format on

public:
  /// RV364F: standard constructor
  RV64F( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "RV64F", Feature, RevMem, Output ) {
    SetTable( std::move( RV64FTable ) );
  }
};  // end class RV64F

}  // namespace SST::RevCPU

#endif
