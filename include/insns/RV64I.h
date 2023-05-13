//
// _RV64I_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64I_H_
#define _SST_REVCPU_RV64I_H_

#include "../RevInstTable.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64I : public RevExt {

      // Compressed instructions
      static bool cldsp(RevFeature *F, RevRegFile *R,
                        RevMem *M, RevInst Inst) {
        // c.ldsp rd, $imm = lw rd, x2, $imm
        Inst.rs1  = 2;
        //ZEXT(Inst.imm, ((Inst.imm&0b111111))*8, 32);
        Inst.imm = ((Inst.imm & 0b111111)*8);

        return ld(F,R,M,Inst);
      }

      static bool csdsp(RevFeature *F, RevRegFile *R,
                        RevMem *M, RevInst Inst) {
        // c.swsp rs2, $imm = sw rs2, x2, $imm
        Inst.rs1  = 2;
        //ZEXT(Inst.imm, ((Inst.imm&0b111111))*8, 32);
        Inst.imm = ((Inst.imm & 0b111111)*8);

        return sd(F,R,M,Inst);
      }

      static bool cld(RevFeature *F, RevRegFile *R,
                      RevMem *M, RevInst Inst) {
        // c.ld %rd, %rs1, $imm = ld %rd, %rs1, $imm
        Inst.rd  = CRegMap[Inst.rd];
        Inst.rs1 = CRegMap[Inst.rs1];
        //Inst.imm = ((Inst.imm&0b11111)*8);
        Inst.imm = (Inst.imm&0b11111111); //8-bit immd, zero-extended, scaled at decode
        return ld(F,R,M,Inst);
      }

      static bool csd(RevFeature *F, RevRegFile *R,
                      RevMem *M, RevInst Inst) {
        // c.sd rs2, rs1, $imm = sd rs2, $imm(rs1)
        Inst.rs2 = CRegMap[Inst.rs2];
        Inst.rs1 = CRegMap[Inst.rs1];
        Inst.imm = (Inst.imm&0b11111111); //imm is 8-bits, zero extended, decoder pre-aligns bits, no scaling needed

        return sd(F,R,M,Inst);
      }

      static bool caddiw(RevFeature *F, RevRegFile *R,
                         RevMem *M, RevInst Inst) {
        // c.addiw %rd, $imm = addiw %rd, %rd, $imm
        Inst.rs1 = Inst.rd;
        uint64_t tmp = Inst.imm & 0b111111;
        SEXT(Inst.imm, tmp, 6);

        return addiw(F,R,M,Inst);
      }

      static bool caddw(RevFeature *F, RevRegFile *R,
                       RevMem *M, RevInst Inst) {
        // c.addw %rd, %rs2 = addw %rd, %rd, %rs2
        Inst.rd  = CRegMap[Inst.rd];
        Inst.rs1 = Inst.rd;
        Inst.rs2  = CRegMap[Inst.rs2];
        return addw(F,R,M,Inst);
      }

      static bool csubw(RevFeature *F, RevRegFile *R,
                       RevMem *M, RevInst Inst) {
        // c.subw %rd, %rs2 = subw %rd, %rd, %rs2
        Inst.rd  = CRegMap[Inst.rd];
        Inst.rs1 = Inst.rd;
        Inst.rs2  = CRegMap[Inst.rs2];
        return subw(F,R,M,Inst);
      }

      // Standard instructions
      static bool lwu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst){
        //ZEXT(R->RV64[Inst.rd],M->ReadU64( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
        uint32_t val = 0;
        M->ReadVal((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))),
                    &val,
                    REVMEM_FLAGS(RevCPU::RevFlag::F_ZEXT64));
        ZEXT(R->RV64[Inst.rd], (uint64_t)val, 64);
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool ld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        //R->RV64[Inst.rd] = M->ReadU64( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
        M->ReadVal((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))),
                    &R->RV64[Inst.rd],
                    REVMEM_FLAGS(0x00));
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
        R->RV64[Inst.rd] = dt_u32((int32_t)(td_u32(R->RV64[Inst.rs1],32)) + (int32_t)(td_u32(Inst.imm,12)),32);
        R->RV64[Inst.rd] &= MASK32;
        SEXTI( R->RV64[Inst.rd], 32 );
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool slliw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        //SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (Inst.imm&0b111111))&MASK32,64);
        SEXT(R->RV64[Inst.rd], ((R->RV64[Inst.rs1]<< (Inst.imm & 0b0111111))&0xffffffff), 32);
        SEXTI(R->RV64[Inst.rd],64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool srliw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t srcTrunc = R->RV64[Inst.rs1] & MASK32;  //Force operation on 32-bit unsigned value
        ZEXT(R->RV64[Inst.rd],(srcTrunc >> (Inst.imm&0b111111))&MASK32,64);
        SEXTI(R->RV64[Inst.rd],32);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sraiw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = R->RV64[Inst.rs1] & int64_t(1<<31);  //Grab sign bit
        int32_t srcTrunc = R->RV64[Inst.rs1] & MASK32;  //Force operation on 32-bit signed value
        SEXT(R->RV64[Inst.rd],((srcTrunc >> (Inst.imm&0b1111111)))|tmp,32);
        SEXTI(R->RV64[Inst.rd],32);
        //replicate rs1[31] across all upper bits
        R->RV64[Inst.rd] = (R->RV64[Inst.rs1] & 0x80000000) ? (R->RV64[Inst.rd] & MASK32) | (0xFFFFFFFF00000000) : (R->RV64[Inst.rd] & MASK32) ;
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool addw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = dt_u32(td_u32(R->RV64[Inst.rs1],32) + td_u32(R->RV64[Inst.rs2],32),32); //addw operates on and produces 32-bit results even on RV64I codes 
        SEXTI(R->RV64[Inst.rd], 32);    //Sign extend the result up to 64bits
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool subw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = dt_u32(td_u32(R->RV64[Inst.rs1],32) - td_u32(R->RV64[Inst.rs2],32),32);
        SEXTI(R->RV64[Inst.rd], 32);    //Sign extend the result up to 64bits
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sllw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (R->RV64[Inst.rs2]&0b11111))&MASK32,64);
        SEXTI(R->RV64[Inst.rd], 32);    //Sign extend the result up to 64bits
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool srlw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t srcTrunc = R->RV64[Inst.rs1] & MASK32;  //Force operation on 32-bit unsigned value, scale up to 64bit to avoid sign bit shift
        R->RV64[Inst.rd] = (srcTrunc >> (R->RV64[Inst.rs2]&0b11111));
        SEXTI(R->RV64[Inst.rd],32);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool sraw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = R->RV64[Inst.rs1] & (1<<31);
        int32_t srcTrunc = R->RV64[Inst.rs1] & MASK32;  //Force operation on 32-bit signed value
        SEXT(R->RV64[Inst.rd],((srcTrunc >> (R->RV64[Inst.rs2]&0b111111)))|tmp,32);
        SEXTI(R->RV64[Inst.rd],32);
        //replicate rs1[31] across all upper bits
        R->RV64[Inst.rd] = (R->RV64[Inst.rs1] & 0x80000000) ? (R->RV64[Inst.rd] & MASK32) | (0xFFFFFFFF00000000) : (R->RV64[Inst.rd] & MASK32) ;
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
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slliw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&slliw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srliw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeR).SetImplFunc(&srliw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sraiw %rd, %rs1, $imm").SetCost(1).SetOpcode( 0b0011011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeR).SetImplFunc(&sraiw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("addw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b000).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&addw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("subw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b000).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&subw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sllw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sllw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srlw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&srlw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sraw %rd, %rs1, %rs2" ).SetCost(1).SetOpcode( 0b0111011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR    ).Setrs3Class(RegUNKNOWN).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sraw ).InstEntry}
      };

    std::vector<RevInstEntry> RV64ICTable = {
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.ldsp %rd, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b011).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&cldsp).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.sdsp %rs2, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b111).Setrs2Class(RegGPR).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCSS).SetImplFunc(&csdsp).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.ld %rd, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b011).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCL).SetImplFunc(&cld).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.sd %rs2, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b111).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCS).SetImplFunc(&csd).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.addiw %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b001).Setrs1Class(RegGPR).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&caddiw).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.addw %rd,%rs1").SetCost(1).SetOpcode(0b01).SetFunct6(0b100111).SetFunct2(0b01).SetrdClass(RegGPR).Setrs2Class(RegGPR).SetFormat(RVCTypeCA).SetImplFunc(&caddw).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.subw %rd,%rs1").SetCost(1).SetOpcode(0b01).SetFunct6(0b100111).SetFunct2(0b00).SetrdClass(RegGPR).Setrs2Class(RegGPR).SetFormat(RVCTypeCA).SetImplFunc(&csubw).SetCompressed(true).InstEntry}
      };

    public:
      /// RV64I: standard constructor
      RV64I( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64I", Feature, RegFile, RevMem, Output ) {
          this->SetTable(RV64ITable);
          this->SetCTable(RV64ICTable);
        }

      /// RV64I: standard destructor
      ~RV64I() { }

    }; // end class RV64I
  } // namespace RevCPU
} // namespace SST

#endif
