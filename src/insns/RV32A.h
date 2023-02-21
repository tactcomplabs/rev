//
// _RV32A_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
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
      class Rev32AInstDefaults : public RevInstDefaults {
        public:
        uint8_t     opcode = 0b0101111;
        uint8_t     funct3 = 0b010;
        RevRegClass rdClass = RegGPR;
        RevRegClass rs1Class = RegGPR;
        RevRegClass rs3Class = RegUNKNOWN;
        uint16_t    imm12 = 0b0;
        RevImmFunc  imm = FUnk;
      };

      std::vector<RevInstEntry> RV32ATable = {
        //bit-25, bit-26 are used for rl and aq, respectively.
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("lr.w %rd, (%rs1)"         ).SetCost(1).SetFunct7( 0b0001000 ).Setrs2Class(RegUNKNOWN).SetImplFunc( &lrw).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("sc.w %rd, %rs1, %rs2"     ).SetCost(1).SetFunct7( 0b0001100 ).Setrs2Class(RegGPR    ).SetImplFunc( &scw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amoswap.w %rd, %rs1, %rs2").SetCost(1).SetFunct7( 0b0000100 ).Setrs2Class(RegGPR    ).SetImplFunc( &amoswapw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amoadd.w %rd, %rs1, %rs2" ).SetCost(1).SetFunct7( 0b0000000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amoaddw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amoxor.w %rd, %rs1, %rs2" ).SetCost(1).SetFunct7( 0b0010000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amoxorw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amoand.w %rd, %rs1, %rs2" ).SetCost(1).SetFunct7( 0b0110000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amoandw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amoor.w %rd, %rs1, %rs2"  ).SetCost(1).SetFunct7( 0b0100000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amoorw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amomin.w %rd, %rs1, %rs2" ).SetCost(1).SetFunct7( 0b1000000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amominw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amomax.w %rd, %rs1, %rs2" ).SetCost(1).SetFunct7( 0b1010000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amomaxw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amominu.w %rd, %rs1, %rs2").SetCost(1).SetFunct7( 0b1100000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amominuw ).InstEntry},
        {RevInstEntryBuilder<Rev32AInstDefaults>().SetMnemonic("amomaxu.w %rd, %rs1, %rs2").SetCost(1).SetFunct7( 0b1110000 ).Setrs2Class(RegGPR    ).SetImplFunc( &amomaxuw ).InstEntry}
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
