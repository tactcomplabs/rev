//
// _RV64A_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64A_H_
#define _SST_REVCPU_RV64A_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>

namespace SST::RevCPU{

class RV64A : public RevExt {

  static bool lrd(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    MemReq req(R->RV64[Inst.rs1], Inst.rd, RevRegClass::RegGPR, F->GetHartToExecID(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete() );
    R->LSQueue->insert( req.LSQHashPair() );
    M->LR(F->GetHartToExecID(),
          R->RV64[Inst.rs1],
          &R->RV64[Inst.rd],
          Inst.aq, Inst.rl, req,
          RevFlag::F_SEXT64);
    R->AdvancePC(Inst);
    return true;
  }

  static bool scd(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    M->SC(F->GetHartToExecID(),
          R->RV64[Inst.rs1],
          &R->RV64[Inst.rs2],
          &R->RV64[Inst.rd],
          Inst.aq, Inst.rl,
          RevFlag::F_SEXT64);
    R->AdvancePC(Inst);
    return true;
  }

  /// Atomic Memory Operations
  template<RevFlag F_AMO>
  static bool amooperd(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    uint32_t flags = static_cast<uint32_t>(F_AMO);

    if( Inst.aq && Inst.rl ){
      flags |= uint32_t(RevFlag::F_AQ) | uint32_t(RevFlag::F_RL);
    }else if( Inst.aq ){
      flags |= uint32_t(RevFlag::F_AQ);
    }else if( Inst.rl ){
      flags |= uint32_t(RevFlag::F_RL);
    }

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
              &R->RV64[Inst.rs2],
              &R->RV64[Inst.rd],
              req,
              RevFlag{flags});

    R->AdvancePC(Inst);

    // update the cost
    R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
    return true;
  }

  static constexpr auto& amoaddd  = amooperd<RevFlag::F_AMOADD>;
  static constexpr auto& amoswapd = amooperd<RevFlag::F_AMOSWAP>;
  static constexpr auto& amoxord  = amooperd<RevFlag::F_AMOXOR>;
  static constexpr auto& amoandd  = amooperd<RevFlag::F_AMOAND>;
  static constexpr auto& amoord   = amooperd<RevFlag::F_AMOOR>;
  static constexpr auto& amomind  = amooperd<RevFlag::F_AMOMIN>;
  static constexpr auto& amomaxd  = amooperd<RevFlag::F_AMOMAX>;
  static constexpr auto& amominud = amooperd<RevFlag::F_AMOMINU>;
  static constexpr auto& amomaxud = amooperd<RevFlag::F_AMOMAXU>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64A Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev64AInstDefaults : RevInstDefaults {
    static constexpr uint8_t     opcode   = 0b0101111;
    static constexpr uint8_t     funct3   = 0b011;
    static constexpr RevRegClass rs2Class = RevRegClass::RegUNKNOWN;
  };
  std::vector<RevInstEntry> RV64ATable = {
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("lr.d %rd, (%rs1)"          ).SetFunct2or7(0b00010).
     Setrs2Class(RevRegClass::RegUNKNOWN).SetImplFunc(&lrd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("sc.d %rd, %rs1, %rs2"      ).SetFunct2or7(0b00011).
     SetImplFunc(&scd ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoswap.d %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b00001).SetImplFunc( &amoswapd ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoadd.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b00000).SetImplFunc( &amoaddd ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoxor.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b00100).SetImplFunc( &amoxord ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoand.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b01100).SetImplFunc( &amoandd ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoor.w %rd, %rs1, %rs2").SetCost(   1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b01000).SetImplFunc( &amoord ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomin.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b10000).SetImplFunc( &amomind ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomax.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b10100).SetImplFunc( &amomaxd ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amominu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b11000).SetImplFunc( &amominud ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomaxu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b011).SetFunct2or7( 0b11100).SetImplFunc( &amomaxud ).InstEntry},
  };


public:
  /// RV64A: standard constructor
  RV64A( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV64A", Feature, RevMem, Output) {
    SetTable(std::move(RV64ATable));
  }
}; // end class RV64A

} // namespace SST::RevCPU

#endif
