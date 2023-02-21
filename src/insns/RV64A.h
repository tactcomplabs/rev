//
// _RV64A_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
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
      class Rev64AInstDefaults : public RevInstDefaults {
        public:
        uint8_t     opcode = 0b0101111;
        uint8_t     funct3 = 0b011;
        RevRegClass rdClass = RegGPR;
        RevRegClass rs1Class = RegGPR;
        RevRegClass rs3Class = RegUNKNOWN;
        uint16_t    imm12 = 0b0;
        RevImmFunc  imm = FUnk;
      };
      std::vector<RevInstEntry> RV64ATable = {
        //bit-25, bit-26 are used for rl and aq, respectively.
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("lr.d %rd, (%rs1)"          ).SetCost(1).SetFunct7(0b0001000).Setrs2Class(RegUNKNOWN).SetImplFunc(&lrd ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("sc.d %rd, %rs1, %rs2"      ).SetCost(1).SetFunct7(0b0001100).Setrs2Class(RegGPR    ).SetImplFunc(&scd ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoswap.d %rd, %rs1, %rs2" ).SetCost(1).SetFunct7(0b0000100).Setrs2Class(RegGPR    ).SetImplFunc(&amoswapd ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoadd.d %rd, %rs1, %rs2"  ).SetCost(1).SetFunct7(0b0000000).Setrs2Class(RegGPR    ).SetImplFunc(&amoaddd ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoxor.d %rd, %rs1, %rs2"  ).SetCost(1).SetFunct7(0b0010000).Setrs2Class(RegGPR    ).SetImplFunc(&amoxord ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoand.d %rd, %rs1, %rs2"  ).SetCost(1).SetFunct7(0b0110000).Setrs2Class(RegGPR    ).SetImplFunc(&amoandd ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amoor.d %rd, %rs1, %rs2"   ).SetCost(1).SetFunct7(0b0100000).Setrs2Class(RegGPR    ).SetImplFunc(&amoord ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amomin.d %rd, %rs1, %rs2"  ).SetCost(1).SetFunct7(0b1000000).Setrs2Class(RegGPR    ).SetImplFunc(&amomind ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amomax.d %rd, %rs1, %rs2"  ).SetCost(1).SetFunct7(0b1010000).Setrs2Class(RegGPR    ).SetImplFunc(&amomaxd ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amominu.d %rd, %rs1, %rs2" ).SetCost(1).SetFunct7(0b1100000).Setrs2Class(RegGPR    ).SetImplFunc(&amominud ).InstEntry},
        {RevInstEntryBuilder<Rev64AInstDefaults>().SetMnemonic("amomaxu.d %rd, %rs1, %rs2" ).SetCost(1).SetFunct7(0b1110000).Setrs2Class(RegGPR    ).SetImplFunc(&amomaxud ).InstEntry}
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
