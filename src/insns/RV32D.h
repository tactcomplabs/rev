//
// _RV32D_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
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

      // Compressed instructions
      static bool cfldsp(RevFeature *F, RevRegFile *R,
                        RevMem *M, RevInst Inst) {
        // c.flwsp rd, $imm = lw rd, x2, $imm
        Inst.rs1  = 2;

        return fld(F,R,M,Inst);
      }

      static bool cfsdsp(RevFeature *F, RevRegFile *R,
                        RevMem *M, RevInst Inst) {
        // c.fsdsp rs2, $imm = fsd rs2, x2, $imm
        Inst.rs1  = 2;

        return fsd(F,R,M,Inst);
      }

      static bool cfld(RevFeature *F, RevRegFile *R,
                       RevMem *M, RevInst Inst) {
        // c.fld %rd, %rs1, $imm = flw %rd, %rs1, $imm
        Inst.rd  = CRegMap[Inst.rd];
        Inst.rs1 = CRegMap[Inst.rs1];

        return fld(F,R,M,Inst);
      }

      static bool cfsd(RevFeature *F, RevRegFile *R,
                       RevMem *M, RevInst Inst) {
        // c.fsd rs2, rs1, $imm = fsd rs2, $imm(rs1)
        Inst.rs2 = CRegMap[Inst.rd];
        Inst.rs1 = CRegMap[Inst.rs1];

        return fsd(F,R,M,Inst);
      }

      // Standard instructions
      static bool fld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t rval;
        if( F->IsRV32() ){
          rval = M->ReadU64((uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
          R->RV32_PC += Inst.instSize;
        }else{
          rval = M->ReadU64((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
          R->RV64_PC += Inst.instSize;
        }
        R->DFP[Inst.rd] = rval;
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool fsd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU64((uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), R->DFP[Inst.rs2]);
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), R->DFP[Inst.rs2]);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmaddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = fma_sf64(R->DFP[Inst.rs1], 
                                   R->DFP[Inst.rs2],
                                   R->DFP[Inst.rs3], 
                                   rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmsubd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = fma_sf64(R->DFP[Inst.rs1], 
                                   R->DFP[Inst.rs2],
                                   R->DFP[Inst.rs3] ^ FSIGN_MASK64, 
                                   rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fnmsubd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = fma_sf64(R->DFP[Inst.rs1] ^ FSIGN_MASK64, 
                                   R->DFP[Inst.rs2],
                                   R->DFP[Inst.rs3], 
                                   rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fnmaddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = fma_sf64(R->DFP[Inst.rs1] ^ FSIGN_MASK64, 
                                   R->DFP[Inst.rs2],
                                   R->DFP[Inst.rs3] ^ FSIGN_MASK64, 
                                   rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool faddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = add_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsubd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = sub_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmuld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = mul_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fdivd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = div_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsqrtd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = sqrt_sf64(R->DFP[Inst.rs1], rm, &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsgnjd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DFP[Inst.rd] = (R->DFP[Inst.rs1] & ~FSIGN_MASK64) |
                          (R->DFP[Inst.rs2] & FSIGN_MASK64);

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsgnjnd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DFP[Inst.rd] = (R->DFP[Inst.rs1] & ~FSIGN_MASK64) |
                         ((R->DFP[Inst.rs2] & FSIGN_MASK64) ^ FSIGN_MASK64);

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fsgnjxd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DFP[Inst.rd] = R->DFP[Inst.rs1] ^
                         (R->DFP[Inst.rs2] & FSIGN_MASK64);

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmind(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DFP[Inst.rd] = min_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], &R->fflags);

        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fmaxd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DFP[Inst.rd] = max_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtds(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = cvt_sf32_sf64(R->DFP[Inst.rs1], &R->fflags);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtsd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        R->DFP[Inst.rd] = cvt_sf64_sf32(R->DFP[Inst.rs1], rm, &R->fflags) | ((uint64_t)-1 << 32);
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool feqd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t val;
        val = eq_quiet_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], &R->fflags);
        if( F->IsRV32() ){
          if(Inst.rd != 0)
            R->RV32[Inst.rd] = val;
          R->RV32_PC += Inst.instSize;
        }else{
          if(Inst.rd != 0)
            R->RV64[Inst.rd] = val;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fltd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t val;
        val = lt_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], &R->fflags);
        if( F->IsRV32() ){
          if(Inst.rd != 0)
            R->RV32[Inst.rd] = val;
          R->RV32_PC += Inst.instSize;
        }else{
          if(Inst.rd != 0)
            R->RV64[Inst.rd] = val;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fled(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t val;
        val = le_sf64(R->DFP[Inst.rs1], R->DFP[Inst.rs2], &R->fflags);
        if( F->IsRV32() ){
          if(Inst.rd != 0)
            R->RV32[Inst.rd] = val;
          R->RV32_PC += Inst.instSize;
        }else{
          if(Inst.rd != 0)
            R->RV64[Inst.rd] = val;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fclassd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t val;
        val = fclass_sf64(R->DFP[Inst.rs1]);
        if( F->IsRV32() ){
          if(Inst.rd != 0)
            R->RV32[Inst.rd] = val;
          R->RV32_PC += Inst.instSize;
        }else{
          if(Inst.rd != 0)
            R->RV64[Inst.rd] = val;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtwd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t val;
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        val = (int32_t)cvt_sf64_i32(R->DFP[Inst.rs1], rm, &R->fflags);
        if( F->IsRV32() ){
          if(Inst.rd != 0)
            R->RV32[Inst.rd] = val;
          R->RV32_PC += Inst.instSize;
        }else{
          if(Inst.rd != 0)
            R->RV64[Inst.rd] = val;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtwud(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t val;
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        val = (int32_t)cvt_sf64_u32(R->DFP[Inst.rs1], rm, &R->fflags);
        if( F->IsRV32() ){
          if(Inst.rd != 0)
            R->RV32[Inst.rd] = val;
          R->RV32_PC += Inst.instSize;
        }else{
          if(Inst.rd != 0)
            R->RV64[Inst.rd] = val;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtdw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        if( F->IsRV32() ){
          R->DFP[Inst.rd] = cvt_i32_sf64(R->RV32[Inst.rs1], rm, &R->fflags);
          R->RV32_PC += Inst.instSize;
        }else{
          R->DFP[Inst.rd] = cvt_i32_sf64(R->RV64[Inst.rs1], rm, &R->fflags);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fcvtdwu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        RoundingModeEnum rm = static_cast<RoundingModeEnum>(get_insn_rm(R, Inst.rm));
        if( F->IsRV32() ){
          R->DFP[Inst.rd] = cvt_u32_sf64(R->RV32[Inst.rs1], rm, &R->fflags);
          R->RV32_PC += Inst.instSize;
        }else{
          R->DFP[Inst.rd] = cvt_u32_sf64(R->RV64[Inst.rs1], rm, &R->fflags);
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
        RevRegClass rs3Class  = RegFLOAT;
      };
      std::vector<RevInstEntry> RV32DTable = {
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fld %rd, $imm(%rs1)"           ).SetCost(1).SetOpcode( 0b0000111 ).SetFunct3( 0b011 ).SetFunct7( 0b0       ).SetrdClass(RegFLOAT  ).Setrs1Class(RegGPR  ).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI ).SetImplFunc( &fld ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsd %rs2, $imm(%rs1)"          ).SetCost(1).SetOpcode( 0b0100111 ).SetFunct3( 0b011 ).SetFunct7( 0b0       ).SetrdClass(RegIMM    ).Setrs1Class(RegGPR  ).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS ).SetImplFunc( &fsd ).InstEntry},

        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmadd.d %rd, %rs1, %rs2, %rs3" ).SetCost(1).SetOpcode( 0b1000011 ).SetFunct3( 0b0   ).SetFunct7( 0b0000001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegFLOAT  ).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR4).SetImplFunc( &fmaddd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmsub.d %rd, %rs1, %rs2, %rs3" ).SetCost(1).SetOpcode( 0b1000111 ).SetFunct3( 0b0   ).SetFunct7( 0b0000001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegFLOAT  ).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR4).SetImplFunc( &fmsubd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fnmsub.d %rd, %rs1, %rs2, %rs3").SetCost(1).SetOpcode( 0b1001011 ).SetFunct3( 0b0   ).SetFunct7( 0b0000001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegFLOAT  ).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR4).SetImplFunc( &fnmsubd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fnmadd.d %rd, %rs1, %rs2, %rs3").SetCost(1).SetOpcode( 0b1001111 ).SetFunct3( 0b0   ).SetFunct7( 0b0000001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegFLOAT  ).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR4).SetImplFunc( &fnmaddd ).InstEntry},

        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fadd.d %rd, %rs1, %rs2"        ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0000001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR ).SetImplFunc( &faddd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsub.d %rd, %rs1, %rs2"        ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0000101 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR ).SetImplFunc( &fsubd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmul.d %rd, %rs1, %rs2"        ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0001001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR ).SetImplFunc( &fmuld ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fdiv.d %rd, %rs1, %rs2"        ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0001101 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR ).SetImplFunc( &fdivd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsqrt.d %rd, %rs1"             ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0101101 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b010110100000).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fsqrtd ).InstEntry},
        
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnj.d %rd, %rs1, %rs2"       ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b000 ).SetFunct7( 0b0010001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fsgnjd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnjn.d %rd, %rs1, %rs2"      ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b001 ).SetFunct7( 0b0010001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fsgnjnd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fsgnjx.d %rd, %rs1, %rs2"      ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b010 ).SetFunct7( 0b0010001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fsgnjxd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmin.d %rd, %rs1, %rs2"        ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b000 ).SetFunct7( 0b0010101 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fmind ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fmax.d %rd, %rs1, %rs2"        ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b001 ).SetFunct7( 0b0010101 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fmaxd ).InstEntry},

        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.s.d %rd, %rs1"            ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0100000 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b010000000001).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fcvtsd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.s %rd, %rs1"            ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b0100001 ).SetrdClass(RegFLOAT	).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b010000100000).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fcvtds ).InstEntry},
        
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("feq.d %rd, %rs1, %rs2"         ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b010 ).SetFunct7( 0b1010001 ).SetrdClass(RegGPR    ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &feqd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("flt.d %rd, %rs1, %rs2"         ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b001 ).SetFunct7( 0b1010001 ).SetrdClass(RegGPR    ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fltd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fle.d %rd, %rs1, %rs2"         ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b000 ).SetFunct7( 0b1010001 ).SetrdClass(RegGPR    ).Setrs1Class(RegFLOAT).Setrs2Class(RegFLOAT  ).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc( &fled ).InstEntry},
        
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fclass.d %rd, %rs1"            ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b001 ).SetFunct7( 0b1110001 ).SetrdClass(RegGPR    ).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b111000100000).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fclassd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.w.d %rd, %rs1"            ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b1100001 ).SetrdClass(RegGPR    ).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b110000100000).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fcvtwd ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.wu.d %rd, %rs1"           ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b1100001 ).SetrdClass(RegGPR    ).Setrs1Class(RegFLOAT).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b110000100001).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fcvtwud).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.w %rd, %rs1"            ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b1101001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegGPR  ).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b110100100000).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fcvtdw ).InstEntry},
        {RevInstEntryBuilder<Rev32DInstDefaults>().SetMnemonic("fcvt.d.wu %rd, %rs1"           ).SetCost(1).SetOpcode( 0b1010011 ).SetFunct3( 0b0   ).SetFunct7( 0b1101001 ).SetrdClass(RegFLOAT  ).Setrs1Class(RegGPR  ).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b110100100001).Setimm(FEnc).SetFormat(RVTypeR).SetImplFunc( &fcvtdwu).InstEntry}
      };

    std::vector<RevInstEntry> RV32DCTable = {
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fldsp %rd, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b001).SetrdClass(RegFLOAT).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&cfldsp).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fsdsp %rs1, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b101).Setrs2Class(RegFLOAT).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCSS).SetImplFunc(&cfsdsp).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fld %rd, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b001).Setrs1Class(RegGPR).SetrdClass(RegFLOAT).Setimm(FVal).SetFormat(RVCTypeCL).SetImplFunc(&cfld).SetCompressed(true).InstEntry},
        {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.fsd %rs2, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b101).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCS).SetImplFunc(&cfsd).SetCompressed(true).InstEntry}
      };

    public:
      /// RV32D: standard constructor
      RV32D( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV32D", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV32DTable);
          this->SetCTable(RV32DCTable);
        }

      /// RV32D: standard destructor
      ~RV32D() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
