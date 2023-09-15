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

namespace SST::RevCPU{

class RV64M : public RevExt{
  // 32-bit Multiplication
  static constexpr auto& mulw  = oper<std::multiplies, OpKind::Reg, std::make_unsigned_t, true>;

  // 32-bit Division
  static constexpr auto& divw  = divrem<DivRem::Div, std::make_signed_t,   true>;
  static constexpr auto& divuw = divrem<DivRem::Div, std::make_unsigned_t, true>;

  // 32-bit Remainder
  static constexpr auto& remw  = divrem<DivRem::Rem, std::make_signed_t,   true>;
  static constexpr auto& remuw = divrem<DivRem::Rem, std::make_unsigned_t, true>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64M Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct Rev64MInstDefaults : RevInstDefaults {
    static constexpr uint8_t opcode = 0b0111011;
    static constexpr uint8_t funct7 = 0b0000001;
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

} // namespace SST::RevCPU

#endif
