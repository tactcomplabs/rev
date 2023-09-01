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

#include <vector>
#include <limits>

namespace SST{
  namespace RevCPU{
    class RV64M : public RevExt {

      static bool mulw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
        R->SetX(F, Inst.rd, R->GetX<uint32_t>(F, Inst.rs1) * R->GetX<uint32_t>(F, Inst.rs2));
        R->AdvancePC(F, Inst.instSize);
        return true;
      }

      template<typename i32_t>
      static bool divw_impl(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
        i32_t rs1 = R->GetX<i32_t>(F, Inst.rs1);
        i32_t rs2 = R->GetX<i32_t>(F, Inst.rs2);
        i32_t res = std::numeric_limits<i32_t>::is_signed &&
          rs1 == std::numeric_limits<i32_t>::min() &&
          rs2 == -i32_t{1} ? rs1 : rs2 ? rs1 / rs2 : -i32_t{1};
        // res is cast to signed int32_t so that it will be sign-extended to 64 bits
        // even when i32_t == uint32_t (unsigned)
        R->SetX(F, Inst.rd, static_cast<int32_t>(res));
        R->AdvancePC(F, Inst.instSize);
        return true;
      }

      static constexpr auto& divw  = divw_impl<int32_t>;
      static constexpr auto& divuw = divw_impl<uint32_t>;

      template<typename i32_t>
      static bool remw_impl(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
        i32_t rs1 = R->GetX<i32_t>(F, Inst.rs1);
        i32_t rs2 = R->GetX<i32_t>(F, Inst.rs2);
        i32_t res = std::numeric_limits<i32_t>::is_signed &&
          rs1 == std::numeric_limits<i32_t>::min() &&
          rs2 == -i32_t{1} ? 0 : rs2 ? rs1 % rs2 : rs1;
        // res is cast to signed int32_t so that it will be sign-extended to 64 bits
        // even when i32_t == uint32_t (unsigned)
        R->SetX(F, Inst.rd, static_cast<int32_t>(res));
        R->AdvancePC(F, Inst.instSize);
        return true;
      }

      static constexpr auto& remw  = remw_impl<int32_t>;
      static constexpr auto& remuw = remw_impl<uint32_t>;

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
      {RevInstEntryBuilder<Rev64MInstDefaults>().SetMnemonic("remuw %rd, %rs1, %rs2").SetFunct3(0b111).SetImplFunc(&remuw ).InstEntry},
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
      ~RV64M() = default;

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST

#endif
