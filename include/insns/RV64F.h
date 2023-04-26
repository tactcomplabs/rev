//
// _RV64F_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64F_H_
#define _SST_REVCPU_RV64F_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64F : public RevExt {

      static bool fcvtls(RevFeature *F, RevRegFile &R,RevMem *M,RevInst Inst) {
        if( F->IsRV64D() ){
          R.RV64[Inst.rd] = (int64_t)((float)(R.DPF[Inst.rs1]));
        }else{
          R.RV64[Inst.rd] = (int64_t)((float)(R.SPF[Inst.rs1]));
        }
        R.RV64_PC += Inst.instSize;
        return true;
      }

      static bool fcvtlus(RevFeature *F, RevRegFile &R,RevMem *M,RevInst Inst) {
        if( F->IsRV64D() ){
          R.RV64[Inst.rd] = (uint64_t)((float)(R.DPF[Inst.rs1]));
        }else{
          R.RV64[Inst.rd] = (uint64_t)((float)(R.SPF[Inst.rs1]));
        }
        R.RV64_PC += Inst.instSize;
        return true;
      }

      static bool fcvtsl(RevFeature *F, RevRegFile &R,RevMem *M,RevInst Inst) {
        if( F->IsRV64D() ){
          R.DPF[Inst.rd] = (float)((int64_t)(R.RV64[Inst.rs1]));
        }else{
          R.SPF[Inst.rd] = (float)((int64_t)(R.RV64[Inst.rs1]));
        }
        R.RV64_PC += Inst.instSize;
        return true;
      }

      static bool fcvtslu(RevFeature *F, RevRegFile &R,RevMem *M,RevInst Inst) {
        if( F->IsRV64D() ){
          R.DPF[Inst.rd] = (float)((uint64_t)(R.RV64[Inst.rs1]));
        }else{
          R.SPF[Inst.rd] = (float)((uint64_t)(R.RV64[Inst.rs1]));
        }
        R.RV64_PC += Inst.instSize;
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV64F Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      class Rev64FInstDefaults : public RevInstDefaults {
        public:
        uint8_t opcode = 0b1010011;
        RevRegClass rdClass = RegFLOAT;
        RevRegClass rs1Class = RegFLOAT;
        RevRegClass rs2Class = RegUNKNOWN;
        RevRegClass rs3Class = RegUNKNOWN;
      };

      std::vector<RevInstEntry > RV64FTable = {
      {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.l.s  %rd, %rs1").SetFunct7( 0b1100000).SetImplFunc(&fcvtls ).InstEntry},
      {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.lu.s %rd, %rs1").SetFunct7( 0b1100000).SetImplFunc(&fcvtlus ).InstEntry},
      {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.s.l %rd, %rs1" ).SetFunct7( 0b1101000).SetImplFunc(&fcvtsl ).InstEntry},
      {RevInstEntryBuilder<Rev64FInstDefaults>().SetMnemonic("fcvt.s.lu %rd, %rs1").SetFunct7( 0b1101000).SetImplFunc(&fcvtslu ) .InstEntry}
      };


    public:
      /// RV364F: standard constructor
      RV64F( RevFeature *Feature,
             RevRegFile &RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64F", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV64FTable);
        }

      /// RV64F: standard destructor
      ~RV64F() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
