//
// _RV64D_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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

  static bool fcvtdl(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, static_cast<double>(R->GetX<int64_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fcvtdlu(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetFP(Inst.rd, static_cast<double>(R->GetX<uint64_t>(Inst.rs1)));
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmvxd(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    uint64_t u64;
    double fp = R->GetFP<double, true>(Inst.rs1);
    memcpy(&u64, &fp, sizeof(u64));
    R->SetX(Inst.rd, u64);
    R->AdvancePC(Inst);
    return true;
  }

  static bool fmvdx(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    uint64_t u64 = R->GetX<uint64_t>(Inst.rs1);
    double fp;
    memcpy(&fp, &u64, sizeof(fp));
    R->SetFP(Inst.rd, fp);
    R->AdvancePC(Inst);
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64D Instructions
  //
  // ----------------------------------------------------------------------
  struct Rev64DInstDefaults : RevInstDefaults {
    Rev64DInstDefaults(){
      SetOpcode(0b1010011);
      Setrs2Class(RevRegClass::RegUNKNOWN);
      SetRaiseFPE(true);
    }
  };

  std::vector<RevInstEntry> RV64DTable = {
    { Rev64DInstDefaults().SetMnemonic("fcvt.l.d %rd, %rs1" ).SetFunct2or7(0b1100001).SetImplFunc(fcvtld ).SetrdClass(RevRegClass::RegGPR  ).Setrs1Class(RevRegClass::RegFLOAT).SetfpcvtOp(0b10) },
    { Rev64DInstDefaults().SetMnemonic("fcvt.lu.d %rd, %rs1").SetFunct2or7(0b1100001).SetImplFunc(fcvtlud).SetrdClass(RevRegClass::RegGPR  ).Setrs1Class(RevRegClass::RegFLOAT).SetfpcvtOp(0b11) },
    { Rev64DInstDefaults().SetMnemonic("fcvt.d.l %rd, %rs1" ).SetFunct2or7(0b1101001).SetImplFunc(fcvtdl ).SetrdClass(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).SetfpcvtOp(0b10) },
    { Rev64DInstDefaults().SetMnemonic("fcvt.d.lu %rd, %rs1").SetFunct2or7(0b1101001).SetImplFunc(fcvtdlu).SetrdClass(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).SetfpcvtOp(0b11) },
    { Rev64DInstDefaults().SetMnemonic("fmv.x.d %rd, %rs1"  ).SetFunct2or7(0b1110001).SetImplFunc(fmvxd  ).SetrdClass(RevRegClass::RegGPR  ).Setrs1Class(RevRegClass::RegFLOAT).SetRaiseFPE(false) },
    { Rev64DInstDefaults().SetMnemonic("fmv.d.x %rd, %rs1"  ).SetFunct2or7(0b1111001).SetImplFunc(fmvdx  ).SetrdClass(RevRegClass::RegFLOAT).Setrs1Class(RevRegClass::RegGPR  ).SetRaiseFPE(false) },
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
