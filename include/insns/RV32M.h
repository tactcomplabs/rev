//
// _RV32M_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32M_H_
#define _SST_REVCPU_RV32M_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV32M : public RevExt {

      static uint64_t mulhu_impl(uint64_t A, uint64_t B){
        uint64_t t;
        uint32_t y1, y2, y3;
        uint64_t a0 = (uint32_t)A, a1 = A >> 32;
        uint64_t b0 = (uint32_t)B, b1 = B >> 32;

        t = a1*b0 + ((a0+b0) >> 32);
        y1 = t;
        y2 = t >> 32;

        t = a0+b1 + y1;
        y1 = t;

        t = a1*b1 + y2 + (t >> 32);
        y2 = t;
        y3 = t >> 32;

        return ((uint64_t)y3 << 32) | y2;
      }

      static int64_t mulh_impl(int64_t A, int64_t B){
        int negate = (A < 0) != (B < 0);
        uint64_t res = mulhu_impl(A < 0 ? -A : A, B < 0 ? -B : B);
        return negate ? ~res + (A * B == 0) : res;
      }

      static int64_t mulhsu_impl(int64_t A, uint64_t B){
        int negate = A < 0;
        uint64_t res = mulhu_impl(A < 0 ? -A : A, B);
        return negate ? ~res + (A * B == 0 ) : res;
      }

      static bool mul(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32(td_u32(R->RV32[Inst.rs1],32) * td_u32(R->RV32[Inst.rs2],32),32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u64(td_u64(R->RV64[Inst.rs1],64) * td_u64(R->RV64[Inst.rs2],64),64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool mulh(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32(mulh_impl(td_u32(R->RV32[Inst.rs1],32),td_u32(R->RV32[Inst.rs2],32))>>32,32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u64(mulh_impl(td_u64(R->RV64[Inst.rs1],64),td_u64(R->RV64[Inst.rs2],64)),64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool mulhsu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32((td_u32(R->RV32[Inst.rs1],32)*td_u32(R->RV32[Inst.rs2],32))>>32,32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u64(mulhsu_impl(td_u64(R->RV64[Inst.rs1],32),td_u64(R->RV64[Inst.rs2],32)),32);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool mulhu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          R->RV32[Inst.rd] = dt_u32((td_u32(R->RV32[Inst.rs1],32)*td_u32(R->RV32[Inst.rs2],32))>>32,32);
          R->RV32_PC += Inst.instSize;
        }else{
          R->RV64[Inst.rd] = dt_u64(mulhu_impl(td_u64(R->RV64[Inst.rs1],32),td_u64(R->RV64[Inst.rs2],32)),32);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool div(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          uint32_t lhs = td_u32(R->RV32[Inst.rs1],32);
          uint32_t rhs = td_u32(R->RV32[Inst.rs2],32);
          if( rhs == 0 ){
            R->RV32[Inst.rd] = UINT32_MAX;
            return true;
          }else if( (lhs == INT32_MIN) &&
                    ((int32_t)(rhs) == -1) ){
            R->RV32[Inst.rd] = dt_u32(lhs,32);
            return true;
          }
          R->RV32[Inst.rd] = dt_u32(lhs/rhs,32);
          R->RV32_PC += Inst.instSize;
        }else{
          uint64_t lhs = td_u64(R->RV64[Inst.rs1],64);
          uint64_t rhs = td_u64(R->RV64[Inst.rs2],64);
          if( rhs == 0 ){
            R->RV64[Inst.rd] = UINT64_MAX;
            return true;
          }else if( (lhs == INT64_MIN) &&
                    ((int64_t)(rhs) == -1) ){
            R->RV64[Inst.rd] = dt_u64(lhs,64);
            return true;
          }
          R->RV64[Inst.rd] = dt_u64(lhs/rhs,64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool divu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          uint32_t lhs = R->RV32[Inst.rs1];
          uint32_t rhs = R->RV32[Inst.rs2];
          ZEXTI(lhs,32);
          ZEXTI(rhs,32);
          if( rhs == 0 ){
            R->RV32[Inst.rd] = UINT32_MAX;
            return true;
          }
          SEXT(R->RV32[Inst.rd], lhs/rhs, 32);
          R->RV32_PC += Inst.instSize;
        }else{
          uint64_t lhs = R->RV64[Inst.rs1];
          uint64_t rhs = R->RV64[Inst.rs2];
          ZEXTI64(lhs,64);
          ZEXTI64(rhs,64);
          if( rhs == 0 ){
            R->RV64[Inst.rd] = UINT64_MAX;
            return true;
          }
          SEXT(R->RV64[Inst.rd], lhs/rhs, 64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool rem(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          uint32_t lhs = td_u32(R->RV32[Inst.rs1],32);
          uint32_t rhs = td_u32(R->RV32[Inst.rs2],32);
          SEXTI(lhs,32);
          SEXTI(rhs,32);
          if( rhs == 0 ){
            R->RV32[Inst.rd] = dt_u32(lhs,32);
            return true;
          }else if( (lhs == INT32_MIN) &&
                    ((int32_t)(rhs) == -1) ){
            R->RV32[Inst.rd] = 0;
            return true;
          }
          R->RV32[Inst.rd] = dt_u32(lhs%rhs,32);
          R->RV32_PC += Inst.instSize;
        }else{
          uint64_t lhs = td_u64(R->RV64[Inst.rs1],64);
          uint64_t rhs = td_u64(R->RV64[Inst.rs2],64);
          if( rhs == 0 ){
            R->RV64[Inst.rd] = dt_u64(lhs,64);
            return true;
          }else if( (lhs == INT64_MIN) &&
                    ((int64_t)(rhs) == -1) ){
            R->RV64[Inst.rd] = 0;
            return true;
          }
          R->RV64[Inst.rd] = dt_u64(lhs%rhs,64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool remu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          uint32_t lhs = R->RV32[Inst.rs1];
          uint32_t rhs = R->RV32[Inst.rs2];
          ZEXTI(lhs,32);
          ZEXTI(rhs,32);
          if( rhs == 0 ){
            SEXT(R->RV32[Inst.rd], R->RV32[Inst.rs1], 32);
            return true;
          }
          SEXT(R->RV32[Inst.rd], lhs%rhs, 32);
          R->RV32_PC += Inst.instSize;
        }else{
          uint64_t lhs = R->RV64[Inst.rs1];
          uint64_t rhs = R->RV64[Inst.rs2];
          ZEXTI64(lhs,64);
          ZEXTI64(rhs,64);
          if( rhs == 0 ){
            SEXT(R->RV64[Inst.rd], R->RV64[Inst.rs1], 64);
            return true;
          }
          SEXT(R->RV64[Inst.rd], lhs%rhs, 64);
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV32M Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      class RevMInstDefaults : public RevInstDefaults {
        public:
        uint8_t funct7 = 0b0000001;
        uint8_t opcode = 0b0110011;
      };
      std::vector<RevInstEntry> RV32MTable = {
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mul %rd, %rs1, %rs2"   ).SetFunct3(0b000).SetImplFunc( &mul ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mulh %rd, %rs1, %rs2"  ).SetFunct3(0b001).SetImplFunc( &mulh ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mulhsu %rd, %rs1, %rs2").SetFunct3(0b010).SetImplFunc( &mulhsu ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mulhu %rd, %rs1, %rs2" ).SetFunct3(0b011).SetImplFunc( &mulhu ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("div %rd, %rs1, %rs2"   ).SetFunct3(0b100).SetImplFunc( &div ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("divu %rd, %rs1, %rs2"  ).SetFunct3(0b101).SetImplFunc( &divu ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("rem %rd, %rs1, %rs2"   ).SetFunct3(0b110).SetImplFunc( &rem ).InstEntry},
      {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("remu %rd, %rs1, %rs20" ).SetFunct3(0b111).SetImplFunc( &remu ).InstEntry}
      };

    public:
      /// RV32M: standard constructor
      RV32M( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV32M", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV32MTable);
        }

      /// RV32M: standard destructor
      ~RV32M() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
