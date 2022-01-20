//
// _RV32D_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
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
      class Rev32DInstDefaults : public RevInstDefaults {
        public:
        RevRegClass rdClass   = RegFLOAT;
        RevRegClass rs1Class  = RegFLOAT;
        RevRegClass rs2Class  = RegFLOAT;
      };
      std::vector<RevInstEntry> RV32DTable = {
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fld %rd, $imm(%rs1)"           ).SetOpcode( 0b0000111).SetFunct3(0b011 ).SetFunct7(0b0000000	).SetrdClass(RegFLOAT	).Setrs1Class(RegGPR  ).Setrs2Class(RegGPR).Setrs3Class(    RegUNKNOWN).SetFormat(RVTypeI).SetImplFunc(&fld ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsd %rs2, $imm(%rs1)"          ).SetOpcode( 0b0100111).SetFunct3(0b011 ).SetFunct7(0b0000000	).SetrdClass(RegIMM   ).Setrs1Class(RegFLOAT  ).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeS).SetImplFunc(&fsd ).InstEntry},

      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmadd.d %rd, %rs1, %rs2, %rs3" ).SetOpcode( 0b1000011).SetFunct3(0b0   ).SetFunct7(0b00	    ).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fmaddd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmsub.d %rd, %rs1, %rs2, %rs3" ).SetOpcode( 0b1000111).SetFunct3(0b0   ).SetFunct7(0b00	    ).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fmsubd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fnmsub.d %rd, %rs1, %rs2, %rs3").SetOpcode( 0b1001011).SetFunct3(0b0   ).SetFunct7(0b00	    ).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fnmsubd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fnmadd.d %rd, %rs1, %rs2, %rs3").SetOpcode( 0b1001111).SetFunct3(0b0   ).SetFunct7(0b00	    ).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegFLOAT  ).SetFormat(RVTypeR4).SetImplFunc(&fnmaddd ).InstEntry},

      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fadd.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0000001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&faddd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsub.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0000101	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsubd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmul.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0001001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fmuld ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fdiv.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0001101	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fdivd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsqrt.d %rd, %rs1"             ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0101101	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsqrtd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmin.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b000 ).SetFunct7(0b0010101	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fmind ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmax.d %rd, %rs1, %rs2"        ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b0010101	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fmaxd ).InstEntry},

      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnj.d %rd, %rs1, %rs2"       ).SetOpcode( 0b1010011).SetFunct3(0b000 ).SetFunct7(0b0010001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsgnjd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnjn.d %rd, %rs1, %rs2"      ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b0010001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsgnjnd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnjx.d %rd, %rs1, %rs2"      ).SetOpcode( 0b1010011).SetFunct3(0b010 ).SetFunct7(0b0010001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fsgnjxd ).InstEntry},

      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.s.d %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0100000	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fcvtsd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.s %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b0100001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fcvtsd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("feq.d %rd, %rs1, %rs2"         ).SetOpcode( 0b1010011).SetFunct3(0b010 ).SetFunct7(0b1010001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&feqd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("flt.d %rd, %rs1, %rs2"         ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b1010001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fltd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fle.d %rd, %rs1, %rs2"         ).SetOpcode( 0b1010011).SetFunct3(0b000 ).SetFunct7(0b1010001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT).Setrs3Class(  RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fled ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fclass.d %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b001 ).SetFunct7(0b1110001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fclassd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.w.d %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1100001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fcvtwd ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.wu.d %rd, %rs1"           ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1100001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fcvtwud ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.w %rd, %rs1"            ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1101001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fcvtdw ).InstEntry},
      {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.wu %rd, %rs1"           ).SetOpcode( 0b1010011).SetFunct3(0b0   ).SetFunct7(0b1101001	).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).SetFormat(RVTypeR).SetImplFunc(&fcvtdwu ).InstEntry}
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
