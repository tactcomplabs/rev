//
// _RV32D_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32D_H_
#define _SST_REVCPU_RV32D_H_

#include "RevInstTable.h"
#include "RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV32D : public RevExt {

      static bool fld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->DPF[Inst.rd] = M->ReadDouble((uint64_t)(R->RV32[Inst.rs1]+Inst.imm));
          R->RV32_PC += Inst.instSize;
        }else{
          R->DPF[Inst.rd] = M->ReadDouble((uint64_t)(R->RV64[Inst.rs1]+Inst.imm));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteDouble((uint64_t)(R->RV32[Inst.rs1]+Inst.imm), (double)(R->DPF[Inst.rs2]));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteDouble((uint64_t)(R->RV64[Inst.rs1]+Inst.imm), (double)(R->DPF[Inst.rs2]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmaddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((double)(R->DPF[Inst.rs1]) *
                                   (double)(R->DPF[Inst.rs2]) +
                                   (double)(R->DPF[Inst.rs3]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmsubd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((double)(R->DPF[Inst.rs1]) *
                                   (double)(R->DPF[Inst.rs2]) -
                                   (double)(R->DPF[Inst.rs3]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fnmsubd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((-(double)(R->DPF[Inst.rs1])) *
                                    (double)(R->DPF[Inst.rs2]) -
                                    (double)(R->DPF[Inst.rs3]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fnmaddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((-(double)(R->DPF[Inst.rs1])) *
                                    (double)(R->DPF[Inst.rs2]) +
                                    (double)(R->DPF[Inst.rs3]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool faddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((double)(R->DPF[Inst.rs1]) +
                                   (double)(R->DPF[Inst.rs2]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsubd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((double)(R->DPF[Inst.rs1]) -
                                   (double)(R->DPF[Inst.rs2]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmuld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((double)(R->DPF[Inst.rs1]) *
                                   (double)(R->DPF[Inst.rs2]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fdivd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((double)(R->DPF[Inst.rs1]) /
                                   (double)(R->DPF[Inst.rs2]));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsqrtd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)(sqrt((double)(R->DPF[Inst.rs1])));
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsgnjd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = 0x00ull;
        uint64_t tmp2 = 0x00ull;

        std::memcpy(&tmp,&R->DPF[Inst.rs1],sizeof(uint64_t));
        tmp &= ~(1<<63);
        std::memcpy(&tmp2,&R->DPF[Inst.rs2],sizeof(uint64_t));
        tmp |= (tmp2&(1<<63));
        std::memcpy(&R->DPF[Inst.rd],&tmp,sizeof(double));

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsgnjnd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = 0x00ull;
        uint64_t tmp2 = 0x00ull;

        std::memcpy(&tmp,&R->DPF[Inst.rs1],sizeof(uint64_t));
        tmp &= ~(1<<63);
        std::memcpy(&tmp2,&R->DPF[Inst.rs2],sizeof(uint64_t));
        tmp2 ^= ~(1<<63);
        tmp |= (tmp2&(1<<63));
        std::memcpy(&R->DPF[Inst.rd],&tmp,sizeof(double));

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsgnjxd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t tmp = 0x00ull;
        uint64_t tmp2 = 0x00ull;

        std::memcpy(&tmp,&R->DPF[Inst.rs1],sizeof(uint64_t));
        tmp &= ~(1<<63);
        std::memcpy(&tmp2,&R->DPF[Inst.rs2],sizeof(uint64_t));
        tmp |= ((tmp & (1<<63) )^(tmp2 & (1<<63)));
        std::memcpy(&R->DPF[Inst.rd],&tmp,sizeof(double));

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmind(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        double tmp1 = (double)(R->DPF[Inst.rs1]);
        double tmp2 = (double)(R->DPF[Inst.rs2]);
        if( tmp1 < tmp2 ){
          R->DPF[Inst.rd] = tmp1;
        }else{
          R->DPF[Inst.rd] = tmp2;
        }

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmaxd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        double tmp1 = (double)(R->DPF[Inst.rs1]);
        double tmp2 = (double)(R->DPF[Inst.rs2]);
        if( tmp1 > tmp2 ){
          R->DPF[Inst.rd] = tmp1;
        }else{
          R->DPF[Inst.rd] = tmp2;
        }

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtsd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)(R->DPF[Inst.rs1]);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtds(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (float)(R->DPF[Inst.rs1]);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool feqd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->DPF[Inst.rs1] == R->DPF[Inst.rs1] ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( R->DPF[Inst.rs1] == R->DPF[Inst.rs1] ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fltd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->DPF[Inst.rs1] < R->DPF[Inst.rs1] ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( R->DPF[Inst.rs1] < R->DPF[Inst.rs1] ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fled(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->DPF[Inst.rs1] <= R->DPF[Inst.rs1] ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( R->DPF[Inst.rs1] <= R->DPF[Inst.rs1] ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fclassd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtwd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = (int32_t)((double)(R->DPF[Inst.rs1]));
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = (int32_t)((double)(R->DPF[Inst.rs1]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtwud(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = (uint32_t)((double)(R->DPF[Inst.rs1]));
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = (uint32_t)((double)(R->DPF[Inst.rs1]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtdw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->DPF[Inst.rd] = (double)((int32_t)(R->RV32[Inst.rs1]));
          R->RV32_PC += Inst.instSize;
        }else{
          R->DPF[Inst.rd] = (double)((int32_t)(R->RV64[Inst.rs1]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtdwu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->DPF[Inst.rd] = (double)((uint32_t)(R->RV32[Inst.rs1]));
          R->RV32_PC += Inst.instSize;
        }else{
          R->DPF[Inst.rd] = (double)((uint32_t)(R->RV64[Inst.rs1]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV32D Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      std::vector<RevInstEntry> RV32DTable = {
      {"fld %rd, $imm(%rs1)",  1, 0b0000111, 0b011, 0b0000001, RegFLOAT, RegGPR, RegGPR,   RegUNKNOWN, 0b0, FUnk, RVTypeI, &fld },
      {"fsd %rs2, $imm(%rs1)", 1, 0b0100111, 0b011, 0b0000001, RegIMM,   RegGPR, RegFLOAT, RegUNKNOWN, 0b0, FUnk, RVTypeS, &fsd },

      {"fmadd.d %rd, %rs1, %rs2, %rs3",  1, 0b1000011, 0b0, 0b00, RegFLOAT, RegFLOAT, RegFLOAT, RegFLOAT, 0b0, FUnk, RVTypeR4, &fmaddd },
      {"fmsub.d %rd, %rs1, %rs2, %rs3",  1, 0b1000111, 0b0, 0b00, RegFLOAT, RegFLOAT, RegFLOAT, RegFLOAT, 0b0, FUnk, RVTypeR4, &fmsubd },
      {"fnmsub.d %rd, %rs1, %rs2, %rs3", 1, 0b1001011, 0b0, 0b00, RegFLOAT, RegFLOAT, RegFLOAT, RegFLOAT, 0b0, FUnk, RVTypeR4, &fnmsubd },
      {"fnmadd.d %rd, %rs1, %rs2, %rs3", 1, 0b1001111, 0b0, 0b00, RegFLOAT, RegFLOAT, RegFLOAT, RegFLOAT, 0b0, FUnk, RVTypeR4, &fnmaddd },

      {"fadd.d %rd, %rs1, %rs2", 1, 0b1010011, 0b0,   0b0000001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &faddd },
      {"fsub.d %rd, %rs1, %rs2", 1, 0b1010011, 0b0,   0b0000101, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fsubd },
      {"fmul.d %rd, %rs1, %rs2", 1, 0b1010011, 0b0,   0b0001001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fmuld },
      {"fdiv.d %rd, %rs1, %rs2", 1, 0b1010011, 0b0,   0b0001101, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fdivd },
      {"fsqrt.d %rd, %rs1",      1, 0b1010011, 0b0,   0b0101101, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fsqrtd },
      {"fmin.d %rd, %rs1, %rs2", 1, 0b1010011, 0b000, 0b0010101, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fmind },
      {"fmax.d %rd, %rs1, %rs2", 1, 0b1010011, 0b001, 0b0010101, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fmaxd },

      {"fsgnj.d %rd, %rs1, %rs2",  1, 0b1010011, 0b000, 0b0010001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fsgnjd },
      {"fsgnjn.d %rd, %rs1, %rs2", 1, 0b1010011, 0b001, 0b0010001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fsgnjnd },
      {"fsgnjx.d %rd, %rs1, %rs2", 1, 0b1010011, 0b010, 0b0010001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fsgnjxd },

      {"fcvt.s.d %rd, %rs1",    1, 0b1010011, 0b0,   0b0100000, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fcvtsd },
      {"fcvt.d.s %rd, %rs1",    1, 0b1010011, 0b0,   0b0100001, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fcvtsd },
      {"feq.d %rd, %rs1, %rs2", 1, 0b1010011, 0b010, 0b1010001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &feqd },
      {"flt.d %rd, %rs1, %rs2", 1, 0b1010011, 0b001, 0b1010001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fltd },
      {"fle.d %rd, %rs1, %rs2", 1, 0b1010011, 0b000, 0b1010001, RegFLOAT, RegFLOAT, RegFLOAT,   RegUNKNOWN, 0b0, FUnk, RVTypeR, &fled },
      {"fclass.d %rd, %rs1",    1, 0b1010011, 0b001, 0b1110001, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fclassd },
      {"fcvt.w.d %rd, %rs1",    1, 0b1010011, 0b0,   0b1100001, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fcvtwd },
      {"fcvt.wu.d %rd, %rs1",   1, 0b1010011, 0b0,   0b1100001, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fcvtwud },
      {"fcvt.d.w %rd, %rs1",    1, 0b1010011, 0b0,   0b1101001, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fcvtdw },
      {"fcvt.d.wu %rd, %rs1",   1, 0b1010011, 0b0,   0b1101001, RegFLOAT, RegFLOAT, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &fcvtdwu }
      };


    public:
      /// RV32D: standard constructor
      RV32D( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV32D", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV32DTable);
        }

      /// RV32D: standard destructor
      ~RV32D() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
