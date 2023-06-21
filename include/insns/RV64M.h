//
// _RV64M_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64M_H_
#define _SST_REVCPU_RV64M_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64M : public RevExt {

      static uint64_t mulhu64_impl(uint64_t A, uint64_t B){
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

      static int64_t mulh64_impl(int64_t A, int64_t B){
        int negate = (A < 0) != (B < 0);
        uint64_t res = mulhu64_impl(A < 0 ? -A : A, B < 0 ? -B : B);
        return negate ? ~res + (A * B == 0) : res;
      }

      static int64_t mulhsu64_impl(int64_t A, uint64_t B){
        int negate = A < 0;
        uint64_t res = mulhu64_impl(A < 0 ? -A : A, B);
        return negate ? ~res + (A * B == 0 ) : res;
      }

      static bool mulw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = dt_u64(td_u64(R->RV64[Inst.rs1]&MASK32,32) * td_u64(R->RV64[Inst.rs2]&MASK32,32),32);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool divw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t lhs = td_u64(R->RV64[Inst.rs1] & MASK32,32);
        uint64_t rhs = td_u64(R->RV64[Inst.rs2] & MASK32,32);
        if( rhs == 0 ){
          R->RV64[Inst.rd] = UINT32_MAX;
          R->RV64_PC += Inst.instSize;
          return true;
        }else if( (lhs == INT32_MIN) &&
                  ((int32_t)(rhs) == -1) ){
          R->RV64[Inst.rd] = dt_u64(lhs & MASK32,32);
          R->RV64_PC += Inst.instSize;
          return true;
        }
        R->RV64[Inst.rd] = dt_u64((lhs/rhs)&MASK32,32);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool divuw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t lhs = R->RV64[Inst.rs1] & MASK32;
        uint64_t rhs = R->RV64[Inst.rs2] & MASK32;
        ZEXTI(lhs,64);
        ZEXTI(rhs,64);
        if( rhs == 0 ){
          R->RV64[Inst.rd] = UINT32_MAX;
          return true;
        }
        SEXT(R->RV64[Inst.rd], (lhs/rhs)&MASK32, 64);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool remw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t lhs = td_u64(R->RV64[Inst.rs1] & MASK32,32);
        uint64_t rhs = td_u64(R->RV64[Inst.rs2] & MASK32,32);
        if( rhs == 0 ){
          R->RV64[Inst.rd] = UINT32_MAX;
          return true;
        }else if( (lhs == INT32_MIN) &&
                  ((int32_t)(rhs) == -1) ){
          R->RV64[Inst.rd] = 0;
          return true;
        }
        R->RV64[Inst.rd] = dt_u64((lhs%rhs)&MASK32,32);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool remuw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint64_t lhs = R->RV64[Inst.rs1] & MASK32;
        uint64_t rhs = R->RV64[Inst.rs2] & MASK32;
        ZEXTI(lhs,64);
        ZEXTI(rhs,64);
        if( rhs == 0 ){
          SEXT(R->RV64[Inst.rd], R->RV64[Inst.rs1]&MASK32, 32);
          R->RV64[Inst.rd] = UINT32_MAX;
          return true;
        }
        SEXT(R->RV64[Inst.rd], (lhs%rhs)&MASK32, 32);
        R->RV64_PC += Inst.instSize;
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV64M Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      class Rev64MInstDefaults : public RevInstDefaults {
        public:
        uint8_t opcode = 0b0111011;
        uint8_t funct7 = 0b0000001;
      };
      std::vector<RevInstEntry > RV64MTable = {
      {RevInstEntryBuilder<Rev64MInstDefaults>().SetMnemonic("mulw %rd, %rs1, %rs2" ).SetFunct3(0b000).SetImplFunc(&mulw ).InstEntry},
      {RevInstEntryBuilder<Rev64MInstDefaults>().SetMnemonic("divw %rd, %rs1, %rs2" ).SetFunct3(0b100).SetImplFunc(&divw ).InstEntry},
      {RevInstEntryBuilder<Rev64MInstDefaults>().SetMnemonic("divuw %rd, %rs1, %rs2").SetFunct3(0b101).SetImplFunc(&divuw ).InstEntry},
      {RevInstEntryBuilder<Rev64MInstDefaults>().SetMnemonic("remw %rd, %rs1, %rs2" ).SetFunct3(0b110).SetImplFunc(&remw ).InstEntry},
      {RevInstEntryBuilder<Rev64MInstDefaults>().SetMnemonic("remuw %rd, %rs1, %rs2").SetFunct3(0b111).SetImplFunc(&remuw ).InstEntry}
      };


    public:
      /// RV64M: standard constructor
      RV64M( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64M", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV64MTable);
        }

      /// RV64M: standard destructor
      ~RV64M() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
