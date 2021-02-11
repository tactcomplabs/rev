//
// _RV32A_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32A_H_
#define _SST_REVCPU_RV32A_H_

#include "RevInstTable.h"
#include "RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV32A : public RevExt {

      static bool lrw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1])), 32 );
          if( !M->LR(F->GetHart(), (uint64_t)(R->RV32[Inst.rs1])) )
            return false;
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
          if( !M->LR(F->GetHart(), (uint64_t)(R->RV64[Inst.rs1])) )
            return false;
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool scw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          if( M->SC(F->GetHart(), (uint64_t)(R->RV32[Inst.rs1])) ){
            // successfully cleared the reservation
            M->WriteU32( (uint64_t)(R->RV32[Inst.rs1]), (uint32_t)(R->RV32[Inst.rs2]) );
            R->RV32[Inst.rd] = 0;
            return true;
          }else{
            // failed to clear the reservation
            R->RV32[Inst.rd] = 1;
            return true;
          }
        }else{
          if( M->SC(F->GetHart(), (uint64_t)(R->RV64[Inst.rs1])) ){
            // successfully cleared the reservation
            M->WriteU32( (uint64_t)(R->RV64[Inst.rs1]), (uint32_t)(R->RV64[Inst.rs2]) );
            R->RV64[Inst.rd] = 0;
            return true;
          }else{
            // failed to clear the reservation
            R->RV64[Inst.rd] = 1;
            return true;
          }
        }
      }

      static bool amoswapw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1])), 32 );
          M->WriteU32((uint64_t)(R->RV32[Inst.rs1]), (uint32_t)(R->RV32[Inst.rs2]));
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
          M->WriteU32((uint64_t)(R->RV64[Inst.rs1]), (uint32_t)(R->RV64[Inst.rs2]));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoaddw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV32[Inst.rd],32))+
                             (int32_t)(td_u32(R->RV32[Inst.rs2],32)),32));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV64[Inst.rd],32))+
                             (int32_t)(td_u32(R->RV64[Inst.rs2],32)),32));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoxorw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV32[Inst.rd],32))^
                             (int32_t)(td_u32(R->RV32[Inst.rs2],32)),32));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV64[Inst.rd],32))^
                             (int32_t)(td_u32(R->RV64[Inst.rs2],32)),32));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoandw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV32[Inst.rd],32))&
                             (int32_t)(td_u32(R->RV32[Inst.rs2],32)),32));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV64[Inst.rd],32))&
                             (int32_t)(td_u32(R->RV64[Inst.rs2],32)),32));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoorw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV32[Inst.rd],32))|
                             (int32_t)(td_u32(R->RV32[Inst.rs2],32)),32));
          R->RV32_PC += Inst.instSize;
        }else{
          M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                      dt_u32((int32_t)(td_u32(R->RV64[Inst.rd],32))|
                             (int32_t)(td_u32(R->RV64[Inst.rs2],32)),32));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amominw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1])), 32 );
          if( (int32_t)(td_u32(R->RV32[Inst.rd],32)) < (int32_t)(td_u32(R->RV32[Inst.rs2],32)) ){
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rs2]));
          }
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
          if( (int32_t)(td_u32(R->RV64[Inst.rd],32)) < (int32_t)(td_u32(R->RV64[Inst.rs2],32)) ){
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rs2]));
          }
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomaxw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1])), 32 );
          if( (int32_t)(td_u32(R->RV32[Inst.rd],32)) > (int32_t)(td_u32(R->RV32[Inst.rs2],32)) ){
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rs2]));
          }
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
          if( (int32_t)(td_u32(R->RV64[Inst.rd],32)) > (int32_t)(td_u32(R->RV64[Inst.rs2],32)) ){
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rs2]));
          }
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amominuw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1])), 32 );
          if( (uint32_t)(R->RV32[Inst.rd]) < (uint32_t)(R->RV32[Inst.rs2]) ){
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rs2]));
          }
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
          if( (uint32_t)(R->RV64[Inst.rd]) < (uint32_t)(R->RV64[Inst.rs2]) ){
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rs2]));
          }
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomaxuw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          SEXT(R->RV32[Inst.rd],M->ReadU32( (uint64_t)(R->RV32[Inst.rs1])), 32 );
          if( (uint32_t)(R->RV32[Inst.rd]) > (uint32_t)(R->RV32[Inst.rs2]) ){
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV32[Inst.rs1]),
                        (uint32_t)(R->RV32[Inst.rs2]));
          }
          R->RV32_PC += Inst.instSize;
        }else{
          SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
          if( (uint32_t)(R->RV64[Inst.rd]) > (uint32_t)(R->RV64[Inst.rs2]) ){
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rd]));
          }else{
            M->WriteU32((uint64_t)(R->RV64[Inst.rs1]),
                        (uint32_t)(R->RV64[Inst.rs2]));
          }
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV32A Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      std::vector<RevInstEntry> RV32ATable = {
      {"lr.w %rd, (%rs1)",          1, 0b0101111, 0b010, 0b00010, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &lrw },
      {"sc.w %rd, %rs1, %rs2",      1, 0b0101111, 0b010, 0b00011, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &scw },
      {"amoswap.w %rd, %rs1, %rs2", 1, 0b0101111, 0b010, 0b00001, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoswapw },
      {"amoadd.w %rd, %rs1, %rs2",  1, 0b0101111, 0b010, 0b00000, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoaddw },
      {"amoxor.w %rd, %rs1, %rs2",  1, 0b0101111, 0b010, 0b00100, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoxorw },
      {"amoand.w %rd, %rs1, %rs2",  1, 0b0101111, 0b010, 0b01100, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoandw },
      {"amoor.w %rd, %rs1, %rs2",   1, 0b0101111, 0b010, 0b01000, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoorw },
      {"amomin.w %rd, %rs1, %rs2",  1, 0b0101111, 0b010, 0b10000, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amominw },
      {"amomax.w %rd, %rs1, %rs2",  1, 0b0101111, 0b010, 0b10100, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amomaxw },
      {"amominu.w %rd, %rs1, %rs2", 1, 0b0101111, 0b010, 0b11000, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amominuw },
      {"amomaxu.w %rd, %rs1, %rs2", 1, 0b0101111, 0b010, 0b11100, RegGPR, RegGPR, RegGPR,     RegUNKNOWN, 0b0, FUnk, RVTypeR, &amomaxuw }
      };


    public:
      /// RV32A: standard constructor
      RV32A( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV32A", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV32ATable);
        }

      /// RV32A: standard destructor
      ~RV32A() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
