//
// _RV32I_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32I_H_
#define _SST_REVCPU_RV32I_H_

#include "RevInstTable.h"
#include "RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV32I : public RevExt {

      static bool lui(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = 0x00;
          R->RV32[Inst.rd] = (Inst.imm << 12);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = 0x00;
          R->RV64[Inst.rd] = (Inst.imm << 12);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool auipc(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = 0x00;
          R->RV32[Inst.rd] = (Inst.imm << 12) + dt_u32(R->RV32_PC,32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = 0x00;
          R->RV64[Inst.rd] = (Inst.imm << 12) + dt_u64(R->RV64_PC,64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool jal(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        int64_t tmp;
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32_PC + Inst.instSize;  // PC following return
          R->RV32_PC = (int32_t)(R->RV32_PC) + (int32_t)(td_u32(Inst.imm,20));
          R->RV32[0] = 0x00;  // ensure that x0 = 0
        }else{
          tmp = td_u64(Inst.imm,20);
          R->RV64[Inst.rd] = R->RV64_PC + Inst.instSize;  // PC following return
          R->RV64_PC = (int64_t)(R->RV64_PC) + tmp;
          R->RV64[0] = 0x00ull;  // ensure that x0 = 0
        }
        return true;
      }

      static bool jalr(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32_PC + Inst.instSize;  // PC following return
          R->RV32_PC = (td_u32(R->RV32[Inst.rs1],32) + td_u32(Inst.imm,12)) & ~(1<<0);
          R->RV32[0] = 0x00;  // ensure that x0 = 0
        }else{
          R->RV64[Inst.rd] = R->RV64_PC + Inst.instSize;  // PC following return
          R->RV64_PC = (td_u64(R->RV64[Inst.rs1],64) + td_u64(Inst.imm,12)) & ~(1<<0);
          R->RV64[0] = 0x00ull;  // ensure that x0 = 0
        }
        return true;
      }

      static bool beq(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->RV32[Inst.rs1] == R->RV32[Inst.rs2] ){
            R->RV32_PC = R->RV32_PC + (int32_t)(td_u32(Inst.imm,12));
          }else{
            R->RV32_PC = R->RV32_PC + Inst.instSize;
          }
        }else{
          if( R->RV64[Inst.rs1] == R->RV64[Inst.rs2] ){
            R->RV64_PC = R->RV64_PC + (int64_t)(td_u64(Inst.imm,12));
          }else{
            R->RV64_PC = R->RV64_PC + Inst.instSize;
          }
        }
        return true;
      }

      static bool bne(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
	int64_t tmp;
        if( F->IsRV32() ){
          if( R->RV32[Inst.rs1] != R->RV32[Inst.rs2] ){
            R->RV32_PC = R->RV32_PC + (int32_t)(td_u32(Inst.imm,12));
          }else{
            R->RV32_PC = R->RV32_PC + Inst.instSize;
          }
        }else{
          if( R->RV64[Inst.rs1] != R->RV64[Inst.rs2] ){
            tmp = td_u64(Inst.imm,12);
            R->RV64_PC = R->RV64_PC + tmp;
          }else{
            R->RV64_PC = R->RV64_PC + Inst.instSize;
          }
        }
        return true;
      }

      static bool blt(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( (int32_t)(R->RV32[Inst.rs1]) < (int32_t)(R->RV32[Inst.rs2]) ){
            R->RV32_PC = R->RV32_PC + (int32_t)(td_u32(Inst.imm,12));
          }else{
            R->RV32_PC = R->RV32_PC + Inst.instSize;
          }
        }else{
          if( (int64_t)(R->RV64[Inst.rs1]) < (int64_t)(R->RV64[Inst.rs2]) ){
            R->RV64_PC = R->RV64_PC + (int64_t)(td_u64(Inst.imm,12));
          }else{
            R->RV64_PC = R->RV64_PC + Inst.instSize;
          }
        }
        return true;
      }

      static bool bge(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( (int32_t)(R->RV32[Inst.rs1]) > (int32_t)(R->RV32[Inst.rs2]) ){
            R->RV32_PC = R->RV32_PC + (int32_t)(td_u32(Inst.imm,12));
          }else{
            R->RV32_PC = R->RV32_PC + Inst.instSize;
          }
        }else{
          if( (int64_t)(R->RV64[Inst.rs1]) > (int64_t)(R->RV64[Inst.rs2]) ){
            R->RV64_PC = R->RV64_PC + (int64_t)(td_u64(Inst.imm,12));
          }else{
            R->RV64_PC = R->RV64_PC + Inst.instSize;
          }
        }
        return true;
      }

      static bool bltu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->RV32[Inst.rs1] < R->RV32[Inst.rs2] ){
            R->RV32_PC = R->RV32_PC + (int32_t)(td_u32(Inst.imm,12));
          }else{
            R->RV32_PC = R->RV32_PC + Inst.instSize;
          }
        }else{
          if( R->RV64[Inst.rs1] < R->RV64[Inst.rs2] ){
            R->RV64_PC = R->RV64_PC + (int64_t)(td_u64(Inst.imm,12));
          }else{
            R->RV64_PC = R->RV64_PC + Inst.instSize;
          }
        }
        return true;
      }

      static bool bgeu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->RV32[Inst.rs1] >= R->RV32[Inst.rs2] ){
            R->RV32_PC = R->RV32_PC + (int32_t)(td_u32(Inst.imm,12));
          }else{
            R->RV32_PC = R->RV32_PC + Inst.instSize;
          }
        }else{
          if( R->RV64[Inst.rs1] >= R->RV64[Inst.rs2] ){
            R->RV64_PC = R->RV64_PC + (int64_t)(td_u64(Inst.imm,12));
          }else{
            R->RV64_PC = R->RV64_PC + Inst.instSize;
          }
        }
        return true;
      }

      static bool lb(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU8( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU8( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool lh(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU16( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU16( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool lw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool lbu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          //ZEXT(R->RV32[Inst.rd],M->ReadU8( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32[Inst.rd] = M->ReadU8( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
          R->RV32_PC += Inst.instSize;
        }else{
          //ZEXT(R->RV64[Inst.rd],M->ReadU8( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
          R->RV64[Inst.rd] = M->ReadU8( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool lhu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          //ZEXT(R->RV32[Inst.rd],M->ReadU16( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32[Inst.rd] = M->ReadU16( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
          R->RV32_PC += Inst.instSize;
        }else{
          //ZEXT(R->RV64[Inst.rd],M->ReadU16( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
          R->RV64[Inst.rd] = M->ReadU16( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool sb(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU8((uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), (uint8_t)(R->RV32[Inst.rs2]));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU8((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), (uint8_t)(R->RV64[Inst.rs2]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sh(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU16((uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), (uint16_t)(R->RV32[Inst.rs2]));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU16((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), (uint16_t)(R->RV64[Inst.rs2]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU32((uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), (uint32_t)(R->RV32[Inst.rs2]));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU32((uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12))), (uint32_t)(R->RV64[Inst.rs2]));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool addi(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32((int32_t)(td_u32(R->RV32[Inst.rs1],32)) + (int32_t)(td_u32(Inst.imm,12)),32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u32((int32_t)(td_u32(R->RV64[Inst.rs1],32)) + (int32_t)(td_u32(Inst.imm,12)),32);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool slti(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( (int32_t)(td_u32(R->RV32[Inst.rs1],32)) < (int32_t)(td_u32(Inst.imm,12)) ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( (int64_t)(td_u32(R->RV64[Inst.rs1],32)) < (int64_t)(td_u64(Inst.imm,12)) ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sltiu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t tmp = 0x00;
        SEXT(tmp,Inst.imm,12);
        if( F->IsRV32() ){
          if( R->RV32[Inst.rs1] < tmp ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( R->RV64[Inst.rs1] < tmp ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool xori(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32[Inst.rs1] ^ Inst.imm;
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = R->RV64[Inst.rs1] ^ Inst.imm;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool ori(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32[Inst.rs1] | Inst.imm;
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = R->RV64[Inst.rs1] | Inst.imm;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool andi(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32[Inst.rs1] & Inst.imm;
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = R->RV64[Inst.rs1] & Inst.imm;
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool slli(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] << (Inst.imm&0x3F)),32);
          R->RV32[Inst.rd] = R->RV32[Inst.rs1] << (Inst.imm&0x3F);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = R->RV64[Inst.rs1] << (Inst.imm&0x3F);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool srli(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          //ZEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] >> (Inst.imm&0x3F)),32);
          //SEXTI(R->RV32[Inst.rd],32);
          R->RV32[Inst.rd] = R->RV32[Inst.rs1] >> (Inst.imm&0x3F);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = R->RV64[Inst.rs1] >> (Inst.imm&0x3F);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool srai(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t tmp = R->RV32[Inst.rs1] | (1<<31);
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],((R->RV32[Inst.rs1] >> (Inst.imm&0b111111))|tmp),32);
          SEXTI(R->RV32[Inst.rd],32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],((R->RV64[Inst.rs1] >> (Inst.imm&0b111111))|tmp),64);
          SEXTI(R->RV64[Inst.rd],64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool add(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32(td_u32(R->RV32[Inst.rs1],32) + td_u32(R->RV32[Inst.rs2],32),32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u64(td_u64(R->RV64[Inst.rs1],64) + td_u64(R->RV64[Inst.rs2],64),64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sub(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32(td_u32(R->RV32[Inst.rs1],32) - td_u32(R->RV32[Inst.rs2],32),32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u64(td_u64(R->RV64[Inst.rs1],64) - td_u64(R->RV64[Inst.rs2],64),64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sll(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          //SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] << (R->RV32[Inst.rs2]&0b11111)),32);
          R->RV32[Inst.rd] = (R->RV32[Inst.rs1] << (R->RV32[Inst.rs2]&0b11111));
          R->RV32_PC += Inst.instSize;
        }else{
          //SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (R->RV64[Inst.rs2]&0b11111)),64);
          R->RV64[Inst.rd] = (R->RV64[Inst.rs1] << (R->RV64[Inst.rs2]&0b11111));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool slt(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( (int32_t)(td_u32(R->RV32[Inst.rs1],32)) < (int32_t)(td_u32(R->RV32[Inst.rs2],32)) ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( (int32_t)(td_u32(R->RV64[Inst.rs1],32)) < (int32_t)(td_u32(R->RV64[Inst.rs2],32)) ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sltu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( R->RV32[Inst.rs1] < R->RV32[Inst.rs2] ){
            R->RV32[Inst.rd] = 1;
          }else{
            R->RV32[Inst.rd] = 0;
          }
          R->RV32_PC += Inst.instSize;
        }else{
          if( R->RV64[Inst.rs1] < R->RV64[Inst.rs2] ){
            R->RV64[Inst.rd] = 1;
          }else{
            R->RV64[Inst.rd] = 0;
          }
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool f_xor(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] ^ R->RV32[Inst.rs2]), 32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] ^ R->RV64[Inst.rs2]), 64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool srl(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          ZEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] >> (R->RV32[Inst.rs2]&0b11111)),32);
          SEXTI(R->RV32[Inst.rd],32);
          R->RV32_PC += Inst.instSize;
        }else{
          ZEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] >> (R->RV64[Inst.rs2]&0b11111)),64);
          SEXTI(R->RV64[Inst.rd],64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool sra(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t tmp = R->RV32[Inst.rs1] | (1<<31);
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],((R->RV32[Inst.rs1] >> (R->RV32[Inst.rs2]&0b11111))|tmp),32);
          SEXTI(R->RV32[Inst.rd],32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],((R->RV64[Inst.rs1] >> (R->RV64[Inst.rs2]&0b11111))|tmp),64);
          SEXTI(R->RV64[Inst.rd],64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool f_or(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32[Inst.rs1] | R->RV32[Inst.rs2];
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = R->RV64[Inst.rs1] | R->RV64[Inst.rs2];
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool f_and(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] & R->RV32[Inst.rs2]), 32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] & R->RV64[Inst.rs2]), 64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool fence(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;  // temporarily disabled
      }

      static bool fencei(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;  // temporarily disabled
      }

      static bool ecall(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool ebreak(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool csrrw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool csrrs(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool csrrc(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool csrrwi(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool csrrsi(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool csrrci(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV32I Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      std::vector<RevInstEntry> RV32ITable = {
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lui %rd, $imm"  ).SetCost(1).SetOpcode(0b0110111).SetFunct3(0b0).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeU).SetImplFunc(&lui ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("auipc %rd, $imm").SetCost(1).SetOpcode(0b0010111).SetFunct3(0b0).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeU).SetImplFunc(&auipc ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("jal %rd, $imm"       ).SetCost(1).SetOpcode(0b1101111).SetFunct3(0b0  ).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeJ).SetImplFunc(&jal ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("jalr %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1100111).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&jalr ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("beq %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&beq ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bne %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bne ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("blt %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b100).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&blt ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bge %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b101).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bge ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bltu %rs1, %rs2, $imm").SetCost(1).SetOpcode(0b1100011).SetFunct3(0b110).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bltu ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bgeu %rs1, %rs2, $imm").SetCost(1).SetOpcode(0b1100011).SetFunct3(0b111).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bgeu ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lb %rd, $imm(%rs1)" ).SetCost(1).SetOpcode(0b0000011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lb ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lh %rd, $imm(%rs1)" ).SetCost(1).SetOpcode(0b0000011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lh ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lw %rd, $imm(%rs1)" ).SetCost(1).SetOpcode(0b0000011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lbu %rd, $imm(%rs1)").SetCost(1).SetOpcode(0b0000011).SetFunct3(0b100).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lbu ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lhu %rd, $imm(%rs1)").SetCost(1).SetOpcode(0b0000011).SetFunct3(0b101).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lhu ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sb %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sb ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sh %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sh ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sw %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sw ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("addi %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&addi ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slti %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&slti ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sltiu %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b011).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&sltiu ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("xori %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b100).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&xori ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ori %rd, %rs1, $imm"  ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b110).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&ori ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("andi %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b111).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&andi ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slli %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&slli ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srli %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&srli ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srai %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&srai ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("add %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&add ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sub %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sub ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sll %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sll ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slt %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b010).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&slt ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sltu %rd, %rs1, %rs2").SetCost(1).SetOpcode(0b0110011).SetFunct3(0b011).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sltu ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("xor %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b100).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&f_xor ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srl %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&srl ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sra %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sra ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("or %rd, %rs1, %rs2"  ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b110).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&f_or ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("and %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b111).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&f_and ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("fence"  ).SetCost(1).SetOpcode(0b0001111).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&fence ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("fence.i").SetCost(1).SetOpcode(0b0001111).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeI).SetImplFunc(&fencei ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ecall" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b000000000000).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&ecall ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ebreak").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b000000000001).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&ebreak ).InstEntry},

      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrw %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrs %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrs ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrc %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b011).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrc ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrwi %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b101).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrwi ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrsi %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b110).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrsi ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrci %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b111).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrci ).InstEntry}
      };

    public:
      /// RV32I: standard constructor
      RV32I( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV32I", Feature, RegFile, RevMem, Output ) {
          this->SetTable(RV32ITable);
        }

      /// RV32I: standard destructor
      ~RV32I() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
