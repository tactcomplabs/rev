//
// _RV64A_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64A_H_
#define _SST_REVCPU_RV64A_H_

#include "RevInstTable.h"
#include "RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64A : public RevExt {

      static bool lrd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU64( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        if( !M->LR(F->GetHart(), (uint64_t)(R->RV64[Inst.rs1])) )
          return false;
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool scd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( M->SC(F->GetHart(), (uint64_t)(R->RV64[Inst.rs1])) ){
          // successfully cleared the reservation
          M->WriteU64( (uint64_t)(R->RV64[Inst.rs1]), (uint64_t)(R->RV64[Inst.rs2]) );
          R->RV64[Inst.rd] = 0;
          return true;
        }else{
          // failed to clear the reservation
          R->RV64[Inst.rd] = 1;
          return true;
        }
      }

      static bool amoswapd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        M->WriteU64((uint64_t)(R->RV64[Inst.rs1]), (uint64_t)(R->RV64[Inst.rs2]));
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoaddd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                    dt_u64((int64_t)(td_u64(R->RV64[Inst.rd],64))+
                           (int64_t)(td_u64(R->RV64[Inst.rs2],64)),64));
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoxord(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                    dt_u64((int64_t)(td_u64(R->RV64[Inst.rd],64))^
                           (int64_t)(td_u64(R->RV64[Inst.rs2],64)),64));
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoandd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                    dt_u64((int64_t)(td_u64(R->RV64[Inst.rd],64))&
                           (int64_t)(td_u64(R->RV64[Inst.rs2],64)),64));
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoord(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                    dt_u64((int64_t)(td_u64(R->RV64[Inst.rd],64))|
                           (int64_t)(td_u64(R->RV64[Inst.rs2],64)),64));
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomind(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        if( (int64_t)(td_u64(R->RV64[Inst.rd],64)) <
            (int64_t)(td_u64(R->RV64[Inst.rs2],64)) ){
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rd]));
        }else{
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rs2]));
        }
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomaxd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        if( (int64_t)(td_u64(R->RV64[Inst.rd],64)) >
            (int64_t)(td_u64(R->RV64[Inst.rs2],64)) ){
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rd]));
        }else{
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rs2]));
        }
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amominud(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        if( (uint64_t)(R->RV64[Inst.rd]) < (uint64_t)(R->RV64[Inst.rs2]) ){
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rd]));
        }else{
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rs2]));
        }
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomaxud(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        SEXT(R->RV64[Inst.rd],M->ReadU32( (uint64_t)(R->RV64[Inst.rs1])), 64 );
        if( (uint64_t)(R->RV64[Inst.rd]) > (uint64_t)(R->RV64[Inst.rs2]) ){
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rd]));
        }else{
          M->WriteU64((uint64_t)(R->RV64[Inst.rs1]),
                      (uint64_t)(R->RV64[Inst.rs2]));
        }
        R->RV64_PC += Inst.instSize;
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV64A Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      std::vector<RevInstEntry> RV64ATable = {
      {"lr.d %rd, (%rs1)",          1, 0b0101111, 0b011, 0b00010, RegGPR, RegUNKNOWN, RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &lrd },
      {"sc.d %rd, %rs1, %rs2",      1, 0b0101111, 0b011, 0b00011, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &scd },
      {"amoswap.d %rd, %rs1, %rs2", 1, 0b0101111, 0b011, 0b00001, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoswapd },
      {"amoadd.d %rd, %rs1, %rs2",  1, 0b0101111, 0b011, 0b00000, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoaddd },
      {"amoxor.d %rd, %rs1, %rs2",  1, 0b0101111, 0b011, 0b00100, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoxord },
      {"amoand.d %rd, %rs1, %rs2",  1, 0b0101111, 0b011, 0b01100, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoandd },
      {"amoor.d %rd, %rs1, %rs2",   1, 0b0101111, 0b011, 0b01000, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amoord },
      {"amomin.d %rd, %rs1, %rs2",  1, 0b0101111, 0b011, 0b10000, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amomind },
      {"amomax.d %rd, %rs1, %rs2",  1, 0b0101111, 0b011, 0b10100, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amomaxd },
      {"amominu.d %rd, %rs1, %rs2", 1, 0b0101111, 0b011, 0b11000, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amominud },
      {"amomaxu.d %rd, %rs1, %rs2", 1, 0b0101111, 0b011, 0b11100, RegGPR, RegGPR,     RegUNKNOWN, RegUNKNOWN, 0b0, FUnk, RVTypeR, &amomaxud }
      };


    public:
      /// RV64A: standard constructor
      RV64A( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64A", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV64ATable);
        }

      /// RV64A: standard destructor
      ~RV64A() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
