//
// _RV32A_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
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
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  std::vector<RevInstEntry> RV32ATable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lr.w %rd, (%rs1)").SetCost(          1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b00010).SetImplFunc( &lrw).Setrs2Class(RevRegClass::RegUNKNOWN).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sc.w %rd, %rs1, %rs2").SetCost(      1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b00011).SetImplFunc( &scw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoswap.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b00001).SetImplFunc( &amoswapw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoadd.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b00000).SetImplFunc( &amoaddw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoxor.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b00100).SetImplFunc( &amoxorw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoand.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b01100).SetImplFunc( &amoandw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoor.w %rd, %rs1, %rs2").SetCost(   1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b01000).SetImplFunc( &amoorw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomin.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b10000).SetImplFunc( &amominw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomax.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b10100).SetImplFunc( &amomaxw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amominu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b11000).SetImplFunc( &amominuw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomaxu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct2or7( 0b11100).SetImplFunc( &amomaxuw ).InstEntry},
  };

public:
  /// RV32A: standard constructor
  RV32A( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV32A", Feature, RevMem, Output) {
    SetTable(std::move(RV32ATable));
  }

}; // end class RV32I

} // namespace SST::RevCPU

#endif
