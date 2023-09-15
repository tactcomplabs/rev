//
// _RV64A_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64A_H_
#define _SST_REVCPU_RV64A_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

#include <vector>

namespace SST::RevCPU{

class RV64A : public RevExt {

  static bool lrd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    M->LR(F->GetHart(),
          R->RV64[Inst.rs1],
          &R->RV64[Inst.rd],
          Inst.aq, Inst.rl, Inst.hazard,
          REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT64));
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool scd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    M->SC(F->GetHart(),
          R->RV64[Inst.rs1],
          &R->RV64[Inst.rs2],
          &R->RV64[Inst.rd],
          Inst.aq, Inst.rl,
          REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT64));
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  /// Atomic Memory Operations
  template<RevCPU::RevFlag F_AMO>
  static bool amooperd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    uint32_t flags = static_cast<uint32_t>(F_AMO);

    if( Inst.aq && Inst.rl ){
      flags |= uint32_t(RevCPU::RevFlag::F_AQ) | uint32_t(RevCPU::RevFlag::F_RL);
    }else if( Inst.aq ){
      flags |= uint32_t(RevCPU::RevFlag::F_AQ);
    }else if( Inst.rl ){
      flags |= uint32_t(RevCPU::RevFlag::F_RL);
    }

    M->AMOVal(F->GetHart(),
              R->RV64[Inst.rs1],
              &R->RV64[Inst.rs2],
              &R->RV64[Inst.rd],
              Inst.hazard,
              flags);

    R->AdvancePC(F, Inst.instSize);

    // update the cost
    R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
    return true;
  }

  static constexpr auto& amoaddd  = amooperd<RevCPU::RevFlag::F_AMOADD>;
  static constexpr auto& amoswapd = amooperd<RevCPU::RevFlag::F_AMOSWAP>;
  static constexpr auto& amoxord  = amooperd<RevCPU::RevFlag::F_AMOXOR>;
  static constexpr auto& amoandd  = amooperd<RevCPU::RevFlag::F_AMOAND>;
  static constexpr auto& amoord   = amooperd<RevCPU::RevFlag::F_AMOOR>;
  static constexpr auto& amomind  = amooperd<RevCPU::RevFlag::F_AMOMIN>;
  static constexpr auto& amomaxd  = amooperd<RevCPU::RevFlag::F_AMOMAX>;
  static constexpr auto& amominud = amooperd<RevCPU::RevFlag::F_AMOMINU>;
  static constexpr auto& amomaxud = amooperd<RevCPU::RevFlag::F_AMOMAXU>;

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
    static constexpr RevRegClass rs2Class = RegUNKNOWN;
  };
  std::vector<RevInstEntry> RV64ATable = {
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("lr.d %rd, (%rs1)"          ).SetFunct7(0b00010).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).SetImplFunc(&lrd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("sc.d %rd, %rs1, %rs2"      ).SetFunct7(0b00011                        ).Setrs2Class(RegUNKNOWN).SetImplFunc(&scd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoswap.d %rd, %rs1, %rs2" ).SetFunct7(0b00001                        ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amoswapd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoadd.d %rd, %rs1, %rs2"  ).SetFunct7(0b00000												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amoaddd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoxor.d %rd, %rs1, %rs2"  ).SetFunct7(0b00100												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amoxord ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoand.d %rd, %rs1, %rs2"  ).SetFunct7(0b01100												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amoandd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoor.d %rd, %rs1, %rs2"   ).SetFunct7(0b01000												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amoord ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amomin.d %rd, %rs1, %rs2"  ).SetFunct7(0b10000												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amomind ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amomax.d %rd, %rs1, %rs2"  ).SetFunct7(0b10100												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amomaxd ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amominu.d %rd, %rs1, %rs2" ).SetFunct7(0b11000												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amominud ).InstEntry},
    {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amomaxu.d %rd, %rs1, %rs2" ).SetFunct7(0b11100												 ).Setrs2Class(RegUNKNOWN).SetImplFunc(&amomaxud ).InstEntry},
  };


public:
  /// RV64A: standard constructor
  RV64A( RevFeature *Feature,
         RevRegFile *RegFile,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV64A", Feature, RegFile, RevMem, Output) {
    this->SetTable(RV64ATable);
  }

  /// RV64A: standard destructor
  ~RV64A() = default;

}; // end class RV32I

} // namespace SST::RevCPU

#endif
