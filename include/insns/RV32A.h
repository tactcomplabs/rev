//
// _RV32A_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32A_H_
#define _SST_REVCPU_RV32A_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>

namespace SST::RevCPU{

class RV32A : public RevExt {

  static bool lrw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    if( R->IsRV32 ){
      MemReq req(uint64_t(R->RV32[Inst.rs1]), Inst.rd, RevRegClass::RegGPR, F->GetHartToExecID(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete());
      R->LSQueue->insert( req.LSQHashPair() );
      M->LR(F->GetHartToExecID(), uint64_t(R->RV32[Inst.rs1]),
            &R->RV32[Inst.rd],
            Inst.aq, Inst.rl, req,
            RevFlag::F_SEXT32);
    }else{
      MemReq req(R->RV64[Inst.rs1], Inst.rd, RevRegClass::RegGPR, F->GetHartToExecID(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete());
      R->LSQueue->insert( req.LSQHashPair() );
      M->LR(F->GetHartToExecID(), R->RV64[Inst.rs1],
            reinterpret_cast<uint32_t*>(&R->RV64[Inst.rd]),
            Inst.aq, Inst.rl, req,
            RevFlag::F_SEXT64);
    }
    R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
    R->AdvancePC(Inst);
    return true;
  }

  static bool scw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    if( R->IsRV32 ){
      M->SC(F->GetHartToExecID(), R->RV32[Inst.rs1],
            &R->RV32[Inst.rs2],
            &R->RV32[Inst.rd],
            Inst.aq, Inst.rl,
            RevFlag::F_SEXT32);
    }else{
      M->SC(F->GetHartToExecID(), R->RV64[Inst.rs1],
            reinterpret_cast<uint32_t*>(&R->RV64[Inst.rs2]),
            reinterpret_cast<uint32_t*>(&R->RV64[Inst.rd]),
            Inst.aq, Inst.rl,
            RevFlag::F_SEXT64);
    }
    R->AdvancePC(Inst);
    return true;
  }

  template<RevFlag F_AMO>
  static bool amooper(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    uint32_t flags = static_cast<uint32_t>(F_AMO);

    if( Inst.aq && Inst.rl){
      flags |= uint32_t(RevFlag::F_AQ) | uint32_t(RevFlag::F_RL);
    }else if( Inst.aq ){
      flags |= uint32_t(RevFlag::F_AQ);
    }else if( Inst.rl ){
      flags |= uint32_t(RevFlag::F_RL);
    }

    if( R->IsRV32 ){
      MemReq req(R->RV32[Inst.rs1],
                 Inst.rd,
                 RevRegClass::RegGPR,
                 F->GetHartToExecID(),
                 MemOp::MemOpAMO,
                 true,
                 R->GetMarkLoadComplete());
      R->LSQueue->insert( req.LSQHashPair() );
      M->AMOVal(F->GetHartToExecID(),
                R->RV32[Inst.rs1],
                &R->RV32[Inst.rs2],
                &R->RV32[Inst.rd],
                req,
                RevFlag{flags});
    }else{
      flags |= uint32_t(RevFlag::F_SEXT64);
      MemReq req(R->RV64[Inst.rs1],
                 Inst.rd,
                 RevRegClass::RegGPR,
                 F->GetHartToExecID(),
                 MemOp::MemOpAMO,
                 true,
                 R->GetMarkLoadComplete());
      R->LSQueue->insert( req.LSQHashPair() );
      M->AMOVal(F->GetHartToExecID(),
                R->RV64[Inst.rs1],
                reinterpret_cast<int32_t*>(&R->RV64[Inst.rs2]),
                reinterpret_cast<int32_t*>(&R->RV64[Inst.rd]),
                req,
                RevFlag{flags});
    }
    // update the cost
    R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
    R->AdvancePC(Inst);
    return true;
  }

  static constexpr auto& amoswapw = amooper<RevFlag::F_AMOSWAP>;
  static constexpr auto& amoaddw  = amooper<RevFlag::F_AMOADD>;
  static constexpr auto& amoxorw  = amooper<RevFlag::F_AMOXOR>;
  static constexpr auto& amoandw  = amooper<RevFlag::F_AMOAND>;
  static constexpr auto& amoorw   = amooper<RevFlag::F_AMOOR>;
  static constexpr auto& amominw  = amooper<RevFlag::F_AMOMIN>;
  static constexpr auto& amomaxw  = amooper<RevFlag::F_AMOMAX>;
  static constexpr auto& amominuw = amooper<RevFlag::F_AMOMINU>;
  static constexpr auto& amomaxuw = amooper<RevFlag::F_AMOMAXU>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32A Instructions
  //
  // ----------------------------------------------------------------------
  struct RV32AInstDefaults : RevInstDefaults {
    RV32AInstDefaults(){
      SetOpcode(0b0101111);
      SetFunct3(0b010);
    }
  };

  std::vector<RevInstEntry> RV32ATable = {
    { RV32AInstDefaults().SetMnemonic("lr.w %rd, (%rs1)"         ).SetFunct2or7(0b0000010).SetImplFunc(lrw     )
      .Setrs2Class(RevRegClass::RegUNKNOWN) },
    { RV32AInstDefaults().SetMnemonic("sc.w %rd, %rs1, %rs2"     ).SetFunct2or7(0b0000011).SetImplFunc(scw     ) },
    { RV32AInstDefaults().SetMnemonic("amoswap.w %rd, %rs1, %rs2").SetFunct2or7(0b0000001).SetImplFunc(amoswapw) },
    { RV32AInstDefaults().SetMnemonic("amoadd.w %rd, %rs1, %rs2" ).SetFunct2or7(0b0000000).SetImplFunc(amoaddw ) },
    { RV32AInstDefaults().SetMnemonic("amoxor.w %rd, %rs1, %rs2" ).SetFunct2or7(0b0000100).SetImplFunc(amoxorw ) },
    { RV32AInstDefaults().SetMnemonic("amoand.w %rd, %rs1, %rs2" ).SetFunct2or7(0b0001100).SetImplFunc(amoandw ) },
    { RV32AInstDefaults().SetMnemonic("amoor.w %rd, %rs1, %rs2"  ).SetFunct2or7(0b0001000).SetImplFunc(amoorw  ) },
    { RV32AInstDefaults().SetMnemonic("amomin.w %rd, %rs1, %rs2" ).SetFunct2or7(0b0010000).SetImplFunc(amominw ) },
    { RV32AInstDefaults().SetMnemonic("amomax.w %rd, %rs1, %rs2" ).SetFunct2or7(0b0010100).SetImplFunc(amomaxw ) },
    { RV32AInstDefaults().SetMnemonic("amominu.w %rd, %rs1, %rs2").SetFunct2or7(0b0011000).SetImplFunc(amominuw) },
    { RV32AInstDefaults().SetMnemonic("amomaxu.w %rd, %rs1, %rs2").SetFunct2or7(0b0011100).SetImplFunc(amomaxuw) },
  };

public:
  /// RV32A: standard constructor
  RV32A( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt("RV32A", Feature, RevMem, Output) {
    SetTable(std::move(RV32ATable));
  }

}; // end class RV32I

} // namespace SST::RevCPU

#endif
