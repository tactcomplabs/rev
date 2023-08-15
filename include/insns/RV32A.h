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

#include "../RevInstTable.h"
#include "../RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV32A : public RevExt {

      static bool lrw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->LR(F->GetHart(), (uint64_t)(R->RV32[Inst.rs1]),
                (uint32_t *)(&R->RV32[Inst.rd]),
                Inst.aq, Inst.rl,
                REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT32));
          R->RV32_PC += Inst.instSize;
        }else{
          M->LR(F->GetHart(), (uint64_t)(R->RV64[Inst.rs1]),
                (uint32_t *)(&R->RV64[Inst.rd]),
                Inst.aq, Inst.rl,
                REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT64));
          R->RV64_PC += Inst.instSize;
        }
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool scw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        if( F->IsRV32() ){
          M->SC(F->GetHart(), (uint64_t)(R->RV32[Inst.rs1]),
                (uint32_t *)(&R->RV32[Inst.rs2]),
                (uint32_t *)(&R->RV32[Inst.rd]),
                Inst.aq, Inst.rl,
                REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT32));
          R->RV32_PC += Inst.instSize;
        }else{
          M->SC(F->GetHart(), (uint64_t)(R->RV64[Inst.rs1]),
                (uint32_t *)(&R->RV64[Inst.rs2]),
                (uint32_t *)(&R->RV64[Inst.rd]),
                Inst.aq, Inst.rl,
                REVMEM_FLAGS(RevCPU::RevFlag::F_SEXT64));
          R->RV64_PC += Inst.instSize;
        }
        return true;
      }

      static bool amoswapw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOSWAP) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOSWAP) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOSWAP) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoaddw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOADD) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOADD) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOADD) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoxorw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOXOR) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOXOR) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOXOR) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoandw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOAND) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOAND) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOAND) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amoorw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOOR) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOOR) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOOR) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amominw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMIN) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMIN) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMIN) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomaxw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMAX) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMAX) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMAX) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amominuw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMINU) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMINU) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMINU) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV64_PC += Inst.instSize;
        }
        // update the cost
        R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
        return true;
      }

      static bool amomaxuw(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        uint32_t flags = 0x00ul;

        if( Inst.aq && Inst.rl){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMAXU) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }else if( Inst.aq ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMAXU) |
                  (uint32_t)(RevCPU::RevFlag::F_AQ);
        }else if( Inst.rl ){
          flags = (uint32_t)(RevCPU::RevFlag::F_AMOMAXU) |
                  (uint32_t)(RevCPU::RevFlag::F_RL);
        }

        if( F->IsRV32() ){
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV32[Inst.rs1]),
                    (int32_t *)(&R->RV32[Inst.rs2]),
                    (int32_t *)(&R->RV32[Inst.rd]),
                    REVMEM_FLAGS(flags));
          R->RV32_PC += Inst.instSize;
        }else{
          M->AMOVal(F->GetHart(),
                    (uint64_t)(R->RV64[Inst.rs1]),
                    (int32_t *)(&R->RV64[Inst.rs2]),
                    (int32_t *)(&R->RV64[Inst.rd]),
                    REVMEM_FLAGS(flags));
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
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lr.w %rd, (%rs1)").SetCost(          1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00010).SetImplFunc( &lrw).Setrs2Class(RegUNKNOWN).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sc.w %rd, %rs1, %rs2").SetCost(      1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00011).SetImplFunc( &scw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoswap.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00001).SetImplFunc( &amoswapw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoadd.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00000).SetImplFunc( &amoaddw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoxor.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b00100).SetImplFunc( &amoxorw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoand.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b01100).SetImplFunc( &amoandw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amoor.w %rd, %rs1, %rs2").SetCost(   1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b01000).SetImplFunc( &amoorw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomin.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b10000).SetImplFunc( &amominw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomax.w %rd, %rs1, %rs2").SetCost(  1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b10100).SetImplFunc( &amomaxw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amominu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b11000).SetImplFunc( &amominuw ).InstEntry},
      {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("amomaxu.w %rd, %rs1, %rs2").SetCost( 1).SetOpcode( 0b0101111).SetFunct3(0b010).SetFunct7( 0b11100).SetImplFunc( &amomaxuw ).InstEntry}
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
