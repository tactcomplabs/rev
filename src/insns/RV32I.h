//
// _RV32I_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
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
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = R->RV32_PC + Inst.instSize;  // PC following return
          R->RV32_PC = (int32_t)(R->RV32_PC) + (int32_t)(td_u32(Inst.imm,20));
          R->RV32[0] = 0x00;  // ensure that x0 = 0
        }else{
          R->RV64[Inst.rd] = R->RV64_PC + Inst.instSize;  // PC following return
          R->RV64_PC = (int64_t)(R->RV64_PC) + (int64_t)(td_u32(Inst.imm,20));
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
          ZEXT(R->RV32[Inst.rd],M->ReadU8( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32_PC += Inst.instSize;
        }else{
          ZEXT(R->RV64[Inst.rd],M->ReadU8( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool lhu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          ZEXT(R->RV32[Inst.rd],M->ReadU16( (uint64_t)(R->RV32[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),32);
          R->RV32_PC += Inst.instSize;
        }else{
          ZEXT(R->RV64[Inst.rd],M->ReadU16( (uint64_t)(R->RV64[Inst.rs1]+(int32_t)(td_u32(Inst.imm,12)))),64);
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
          SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] << (Inst.imm&0b11111)),32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (Inst.imm&0b111111)),64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool srli(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          ZEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] >> (Inst.imm&0b11111)),32);
          SEXTI(R->RV32[Inst.rd],32);
          R->RV32_PC += Inst.instSize;
        }else{
          ZEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] >> (Inst.imm&0b111111)),64);
          SEXTI(R->RV64[Inst.rd],64);
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
          SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] << (R->RV32[Inst.rs2]&0b11111)),32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] << (R->RV64[Inst.rs2]&0b11111)),64);
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
          SEXT(R->RV32[Inst.rd],(R->RV32[Inst.rs1] | R->RV32[Inst.rs2]), 32);
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],(R->RV64[Inst.rs1] | R->RV64[Inst.rs2]), 64);
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
      {"lui %rd, $imm",         1, 0b0110111, 0b0,    0b0,       RegGPR,     RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeU, &lui },
      {"auipc %rd, $imm",       1, 0b0010111, 0b0,    0b0,       RegGPR,     RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeU, &auipc },

      {"jal %rd, $imm",         1, 0b1101111, 0b0,    0b0,       RegGPR,     RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeJ, &jal },
      {"jalr %rd, %rs1, $imm",  1, 0b1100111, 0b000,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &jalr },

      {"beq %rs1, %rs2, $imm",  1, 0b1100011, 0b000,  0b0,       RegIMM,     RegGPR,     RegGPR    , RegUNKNOWN, 0b0, FUnk, RVTypeB, &beq },
      {"bne %rs1, %rs2, $imm",  1, 0b1100011, 0b001,  0b0,       RegIMM,     RegGPR,     RegGPR    , RegUNKNOWN, 0b0, FUnk, RVTypeB, &bne },
      {"blt %rs1, %rs2, $imm",  1, 0b1100011, 0b100,  0b0,       RegIMM,     RegGPR,     RegGPR    , RegUNKNOWN, 0b0, FUnk, RVTypeB, &blt },
      {"bge %rs1, %rs2, $imm",  1, 0b1100011, 0b101,  0b0,       RegIMM,     RegGPR,     RegGPR    , RegUNKNOWN, 0b0, FUnk, RVTypeB, &bge },
      {"bltu %rs1, %rs2, $imm", 1, 0b1100011, 0b110,  0b0,       RegIMM,     RegGPR,     RegGPR    , RegUNKNOWN, 0b0, FUnk, RVTypeB, &bltu },
      {"bgeu %rs1, %rs2, $imm", 1, 0b1100011, 0b111,  0b0,       RegIMM,     RegGPR,     RegGPR    , RegUNKNOWN, 0b0, FUnk, RVTypeB, &bgeu },

      {"lb %rd, $imm(%rs1)",    1, 0b0000011, 0b000,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &lb },
      {"lh %rd, $imm(%rs1)",    1, 0b0000011, 0b001,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &lh },
      {"lw %rd, $imm(%rs1)",    1, 0b0000011, 0b010,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &lw },
      {"lbu %rd, $imm(%rs1)",   1, 0b0000011, 0b100,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &lbu },
      {"lhu %rd, $imm(%rs1)",   1, 0b0000011, 0b101,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &lhu },

      {"sb %rs2, $imm(%rs1)",  1, 0b0100011, 0b000,  0b0,       RegIMM,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeS, &sb },
      {"sh %rs2, $imm(%rs1)",  1, 0b0100011, 0b001,  0b0,       RegIMM,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeS, &sh },
      {"sw %rs2, $imm(%rs1)",  1, 0b0100011, 0b010,  0b0,       RegIMM,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeS, &sw },

      {"addi %rd, %rs1, $imm",  1, 0b0010011, 0b000,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &addi },
      {"slti %rd, %rs1, $imm",  1, 0b0010011, 0b010,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &slti },
      {"sltiu %rd, %rs1, $imm", 1, 0b0010011, 0b011,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &sltiu },
      {"xori %rd, %rs1, $imm",  1, 0b0010011, 0b100,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &xori },
      {"ori %rd, %rs1, $imm",   1, 0b0010011, 0b110,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &ori },
      {"andi %rd, %rs1, $imm",  1, 0b0010011, 0b111,  0b0,       RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &andi },

      {"slli %rd, %rs1, $imm",  1, 0b0010011, 0b001,  0b0000000, RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &slli },
      {"srli %rd, %rs1, $imm",  1, 0b0010011, 0b101,  0b0000000, RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &srli },
      {"srai %rd, %rs1, $imm",  1, 0b0010011, 0b101,  0b0100000, RegGPR,     RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &srai },

      {"add %rd, %rs1, %rs2",   1, 0b0110011, 0b000,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &add },
      {"sub %rd, %rs1, %rs2",   1, 0b0110011, 0b000,  0b0100000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &sub },
      {"sll %rd, %rs1, %rs2",   1, 0b0110011, 0b001,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &sll },
      {"slt %rd, %rs1, %rs2",   1, 0b0110011, 0b010,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &slt },
      {"sltu %rd, %rs1, %rs2",  1, 0b0110011, 0b011,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &sltu },
      {"xor %rd, %rs1, %rs2",   1, 0b0110011, 0b100,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &f_xor },
      {"srl %rd, %rs1, %rs2",   1, 0b0110011, 0b101,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk,RVTypeR, &srl },
      {"sra %rd, %rs1, %rs2",   1, 0b0110011, 0b101,  0b0100000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &sra },
      {"or %rd, %rs1, %rs2",    1, 0b0110011, 0b110,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &f_or },
      {"and %rd, %rs1, %rs2",   1, 0b0110011, 0b111,  0b0000000, RegGPR,     RegGPR,     RegGPR,     RegUNKNOWN, 0b0,            FUnk, RVTypeR, &f_and },

      {"fence",   1, 0b0001111, 0b000,  0b0,       RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b0,            FVal, RVTypeI, &fence },
      {"fence.i", 1, 0b0001111, 0b001,  0b0,       RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b0,            FUnk, RVTypeI, &fencei },

      {"ecall",   1, 0b1110011, 0b000,  0b0,       RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b000000000000, FEnc, RVTypeI, &ecall },
      {"ebreak",  1, 0b1110011, 0b000,  0b0,       RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b000000000001, FEnc, RVTypeI, &ebreak },

      {"csrrw %rd, %rs1, $imm",   1, 0b1110011, 0b001,  0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FVal, RVTypeU, &csrrw },
      {"csrrs %rd, %rs1, $imm",   1, 0b1110011, 0b010,  0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FVal, RVTypeU, &csrrs },
      {"csrrc %rd, %rs1, $imm",   1, 0b1110011, 0b011,  0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FVal, RVTypeU, &csrrc },
      {"csrrwi %rd, %rs1, $imm",  1, 0b1110011, 0b101,  0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FVal, RVTypeU, &csrrwi },
      {"csrrsi %rd, %rs1, $imm",  1, 0b1110011, 0b110,  0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FVal, RVTypeU, &csrrsi },
      {"csrrci %rd, %rs1, $imm",  1, 0b1110011, 0b111,  0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FVal, RVTypeU, &csrrci }
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
