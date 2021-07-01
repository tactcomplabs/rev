//
// _RV64I_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64I_H_
#define _SST_REVCPU_RV64I_H_

#include "RevInstTable.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64I : public RevExt {

      static bool lwu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst){
        ZEXT(R->RV64[Inst.rd],M->ReadU64( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool ld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = M->ReadU64( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        int64_t tmp = td_u64(Inst.imm,12);
        M->WriteU64((uint64_t)(R->RV64[Inst.rs1]+tmp), (uint64_t)(R->RV64[Inst.rs2]));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool addiw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = dt_u64((int32_t)(td_u64(R->RV64[Inst.rs1],64)) + (int32_t)(td_u32(Inst.imm,12)),64);
        R->RV64[Inst.rd] &= MASK32;
        SEXTI( R->RV64[Inst.rd], 64 );
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool slliw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (Inst.imm&0b111111))&MASK32,64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool srliw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        ZEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] >> (Inst.imm&0b111111))&MASK32,64);
        SEXTI(R->RV64[Inst.rd],64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sraiw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = R->RV64[Inst.rs1] | (1<<31);
        SEXT(R->RV64[Inst.rd],((R->RV64[Inst.rs1] >> (Inst.imm&0b1111111))&MASK32)|tmp,64);
        SEXTI(R->RV64[Inst.rd],64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool addw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = dt_u64(td_u64(R->RV64[Inst.rs1],64) + td_u64(R->RV64[Inst.rs2],64),64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool subw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = dt_u64(td_u64(R->RV64[Inst.rs1],64) - td_u64(R->RV64[Inst.rs2],64),64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sllw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (R->RV64[Inst.rs2]&0b111111))&MASK32,64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool srlw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        ZEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] >> (R->RV64[Inst.rs2]&0b111111))&MASK32,64);
        SEXTI(R->RV64[Inst.rd],64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sraw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = R->RV64[Inst.rs1] | (1<<31);
        SEXT(R->RV64[Inst.rd],((R->RV64[Inst.rs1] >> (R->RV64[Inst.rs2]&0b111111))&MASK32)|tmp,64);
        SEXTI(R->RV64[Inst.rd],64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV64I Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      std::vector<RevInstEntry > RV64ITable = {
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lwu %rd, $imm(%rs1)"  ).SetCost(1).SetOpcode( 0b0000011).SetFunct3(0b110).SetFunct7(0b0      ).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lwu ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ld %rd, $imm(%rs1)"   ).SetCost(1).SetOpcode( 0b0000011).SetFunct3(0b011).SetFunct7(0b0      ).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&ld ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sd %rs2, $imm(%rs1)"  ).SetCost(1).SetOpcode( 0b0100011).SetFunct3(0b011).SetFunct7(0b0      ).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sd ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("addiw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b000).SetFunct7(0b0      ).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&addiw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slliw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&slliw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srliw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&srliw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sraiw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sraiw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("addw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b000).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&addw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("subw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b000).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&subw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sllw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sllw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srlw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&srlw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sraw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sraw ).InstEntry}
      };

    public:
      /// RV64I: standard constructor
      RV64I( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64I", Feature, RegFile, RevMem, Output ) {
          this->SetTable(RV64ITable);
        }

      /// RV64I: standard destructor
      ~RV64I() { }

    }; // end class RV64I
  } // namespace RevCPU
} // namespace SST

#endif
