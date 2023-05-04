//
// _RV64D_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64D_H_
#define _SST_REVCPU_RV64D_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64D : public RevExt {

      static bool fcvtld(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = (int64_t)((double)(R->DPF[Inst.rs1]));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool fcvtwd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = (int64_t)((double)(R->DPF[Inst.rs1]));
        R->RV64_PC += Inst.instSize;
        return true;
      }


      static bool fcvtlud(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->RV64[Inst.rd] = (uint64_t)((double)(R->DPF[Inst.rs1]));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool fcvtdl(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((int64_t)(R->RV64[Inst.rs1]));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool fcvtdlu(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        R->DPF[Inst.rd] = (double)((uint64_t)(R->RV64[Inst.rs1]));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool fmvxd(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        std::memcpy(&R->RV64[Inst.rd],&R->DPF[Inst.rs1],sizeof(double));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      static bool fmvdx(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        std::memcpy(&R->RV64[Inst.rd],&R->DPF[Inst.rs1],sizeof(double));
        R->RV64_PC += Inst.instSize;
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV64D Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      class Rev64DInstDefaults : public RevInstDefaults {
        public:
        uint8_t opcode = 0b1010011;
        RevRegClass rdClass = RegFLOAT;
        RevRegClass rs1Class = RegFLOAT;
        RevRegClass rs2Class = RegUNKNOWN;
      };
      std::vector<RevInstEntry> RV64DTable = {
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.l.d %rd, %rs1"  ).SetFunct7(0b1100001).SetImplFunc( &fcvtld ).InstEntry},
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.lu.d %rd, %rs1" ).SetFunct7(0b1100001).SetImplFunc( &fcvtlud ).InstEntry},
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.d.l %rd, %rs1"  ).SetFunct7(0b1101001).SetImplFunc( &fcvtdl ).InstEntry},
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.d.lu %rd, %rs1" ).SetFunct7(0b1101001).SetImplFunc( &fcvtdlu ).InstEntry},
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fmv.x.d %rd, %rs1"   ).SetFunct7(0b1110001).SetImplFunc( &fmvxd ).InstEntry},
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fmv.d.x %rd, %rs1"   ).SetFunct7(0b1111001).SetImplFunc( &fmvdx ).InstEntry},
      {RevInstEntryBuilder<Rev64DInstDefaults>().SetMnemonic("fcvt.w.d %rd, %rs1" ).SetFunct7(0b1100001).SetImplFunc( &fcvtwd ).InstEntry}
      };


    public:
      /// RV364D: standard constructor
      RV64D( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64D", Feature, RegFile, RevMem, Output) {
          this->SetTable(RV64DTable);
        }

      /// RV364D: standard destructor
      ~RV64D() { }

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
