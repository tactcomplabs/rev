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

  static bool lrw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    MemReq req;
    if( R->IsRV32 ){
      req.Set(uint64_t(R->RV32[Inst.rs1]), Inst.rd, RevRegClass::RegGPR, F->GetHartToExec(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete());
      R->LSQueue->insert({make_lsq_hash(req.DestReg, req.RegType, req.Hart), req});
      M->LR(F->GetHartToExec(), uint64_t(R->RV32[Inst.rs1]),
            &R->RV32[Inst.rd],
            Inst.aq, Inst.rl, req,
            REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT32));
    }else{
      req.Set(R->RV64[Inst.rs1], Inst.rd, RevRegClass::RegGPR, F->GetHartToExec(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete());
      R->LSQueue->insert({make_lsq_hash(req.DestReg, req.RegType, req.Hart), req});
      M->LR(F->GetHartToExec(), R->RV64[Inst.rs1],
            reinterpret_cast<uint32_t*>(&R->RV64[Inst.rd]),
            Inst.aq, Inst.rl, req,
            REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT64));
    }
    R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
    R->AdvancePC(Inst.instSize);
    return true;
  }

  static bool scw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    if( R->IsRV32 ){
      M->SC(F->GetHartToExec(), R->RV32[Inst.rs1],
            &R->RV32[Inst.rs2],
            &R->RV32[Inst.rd],
            Inst.aq, Inst.rl,
            REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT32));
    }else{
      M->SC(F->GetHartToExec(), R->RV64[Inst.rs1],
            reinterpret_cast<uint32_t*>(&R->RV64[Inst.rs2]),
            reinterpret_cast<uint32_t*>(&R->RV64[Inst.rd]),
            Inst.aq, Inst.rl,
            REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT64));
    }
    R->AdvancePC(Inst.instSize);
    return true;
  }

  template<RevCPU::RevFlag F_AMO>
  static bool amooper(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    uint32_t flags = static_cast<uint32_t>(F_AMO);

    if( Inst.aq && Inst.rl){
      flags |= uint32_t(RevCPU::RevFlag::F_AQ) | uint32_t(RevCPU::RevFlag::F_RL);
    }else if( Inst.aq ){
      flags |= uint32_t(RevCPU::RevFlag::F_AQ);
    }else if( Inst.rl ){
      flags |= uint32_t(RevCPU::RevFlag::F_RL);
    }

    MemReq req;
    if( R->IsRV32 ){
      req.Set(R->RV32[Inst.rs1],
              Inst.rd,
              RevRegClass::RegGPR,
              F->GetHartToExec(),
              MemOp::MemOpAMO,
              true,
              R->GetMarkLoadComplete());
      R->LSQueue->insert({make_lsq_hash(Inst.rd,
                                        RevRegClass::RegGPR,
                                        F->GetHartToExec()), req});
      M->AMOVal(F->GetHartToExec(),
                R->RV32[Inst.rs1],
                &R->RV32[Inst.rs2],
                &R->RV32[Inst.rd],
                req,
                flags);
    }else{
      flags |= uint32_t(RevCPU::RevFlag::F_SEXT64);
      req.Set(R->RV64[Inst.rs1],
              Inst.rd,
              RevRegClass::RegGPR,
              F->GetHartToExec(),
              MemOp::MemOpAMO,
              true,
              R->GetMarkLoadComplete());
      R->LSQueue->insert({make_lsq_hash(Inst.rd,
                                        RevRegClass::RegGPR,
                                        F->GetHartToExec()), req});
      M->AMOVal(F->GetHartToExec(),
                R->RV64[Inst.rs1],
                reinterpret_cast<int32_t*>(&R->RV64[Inst.rs2]),
                reinterpret_cast<int32_t*>(&R->RV64[Inst.rd]),
                req,
                flags);
    }
    // update the cost
    R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
    R->AdvancePC(Inst.instSize);
    return true;
  }

  static constexpr auto& amoswapw = amooper<RevCPU::RevFlag::F_AMOSWAP>;
  static constexpr auto& amoaddw  = amooper<RevCPU::RevFlag::F_AMOADD>;
  static constexpr auto& amoxorw  = amooper<RevCPU::RevFlag::F_AMOXOR>;
  static constexpr auto& amoandw  = amooper<RevCPU::RevFlag::F_AMOAND>;
  static constexpr auto& amoorw   = amooper<RevCPU::RevFlag::F_AMOOR>;
  static constexpr auto& amominw  = amooper<RevCPU::RevFlag::F_AMOMIN>;
  static constexpr auto& amomaxw  = amooper<RevCPU::RevFlag::F_AMOMAX>;
  static constexpr auto& amominuw = amooper<RevCPU::RevFlag::F_AMOMINU>;
  static constexpr auto& amomaxuw = amooper<RevCPU::RevFlag::F_AMOMAXU>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32A Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  std::vector<RevInstEntry> RV32ATable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lr.w %rd, (%rs1)").SetCost(          1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00010).SetImplFunc( &lrw).Setrs2Class(RevRegClass::RegUNKNOWN).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sc.w %rd, %rs1, %rs2").SetCost(      1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00011).SetImplFunc( &scw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoswap.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00001).SetImplFunc( &amoswapw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoadd.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00000).SetImplFunc( &amoaddw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoxor.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00100).SetImplFunc( &amoxorw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoand.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b01100).SetImplFunc( &amoandw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoor.w %rd, %rs1, %rs2").SetCost(   1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b01000).SetImplFunc( &amoorw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomin.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b10000).SetImplFunc( &amominw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomax.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b10100).SetImplFunc( &amomaxw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amominu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b11000).SetImplFunc( &amominuw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomaxu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b11100).SetImplFunc( &amomaxuw ).InstEntry},
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
