//
// _RV64P_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64P_H_
#define _SST_REVCPU_RV64P_H_

#include "RevInstTable.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RV64P : public RevExt {

      static bool future(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        return true;
      }

      static bool rfuture(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        return true;
      }

      static bool sfuture(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
        return true;
      }

      // ----------------------------------------------------------------------
      //
      // RISC-V RV64P Instructions
      //
      // Format:
      // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
      //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
      // ----------------------------------------------------------------------
      std::vector<RevInstEntry> RV64PTable = {
      {"future %rd, $imm(%rs1)",   1, 0b0000011, 0b111, 0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &future},
      {"rfuture %rd, $imm(%rs1)",  1, 0b0000011, 0b101, 0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &rfuture},
      {"sfuture %rd, $imm(%rs1)",  1, 0b0000011, 0b100, 0b0, RegGPR, RegGPR, RegUNKNOWN, RegUNKNOWN, 0b0, FImm, RVTypeI, &sfuture},
      };

    public:
      /// RV64P: standard constructor
      RV64P( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV64P", Feature, RegFile, RevMem, Output ) {
          this->SetTable(RV64PTable);
        }

      /// RV64P: standard destructor
      ~RV64P() { }

    }; // end class RV64I
  } // namespace RevCPU
} // namespace SST

#endif
