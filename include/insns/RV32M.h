//
// _RV32M_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32M_H_
#define _SST_REVCPU_RV32M_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <cstdint>
#include <limits>
#include <vector>
#include <functional>

namespace SST::RevCPU{

class RV32M : public RevExt{
  // Multiplication High instructions based on signedness of arguments
  static constexpr auto& mulh   = uppermul<true,  true>;
  static constexpr auto& mulhu  = uppermul<false, false>;
  static constexpr auto& mulhsu = uppermul<true,  false>;

  /// Computes the LOWER half of multiplication, which does not depend on signedness
  static constexpr auto& mul    = oper<std::multiplies, OpKind::Reg>;

  // Division
  static constexpr auto& div    = divrem<DivRem::Div, std::make_signed_t>;
  static constexpr auto& divu   = divrem<DivRem::Div, std::make_unsigned_t>;

  // Remainder
  static constexpr auto& rem    = divrem<DivRem::Rem, std::make_signed_t>;
  static constexpr auto& remu   = divrem<DivRem::Rem, std::make_unsigned_t>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32M Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  struct RevMInstDefaults : RevInstDefaults {
    static constexpr uint8_t funct7 = 0b0000001;
    static constexpr uint8_t opcode = 0b0110011;
  };

  std::vector<RevInstEntry> RV32MTable = {
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mul %rd, %rs1, %rs2"   ).SetFunct3(0b000).SetImplFunc( &mul ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mulh %rd, %rs1, %rs2"  ).SetFunct3(0b001).SetImplFunc( &mulh ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mulhsu %rd, %rs1, %rs2").SetFunct3(0b010).SetImplFunc( &mulhsu ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("mulhu %rd, %rs1, %rs2" ).SetFunct3(0b011).SetImplFunc( &mulhu ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("div %rd, %rs1, %rs2"   ).SetFunct3(0b100).SetImplFunc( &div ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("divu %rd, %rs1, %rs2"  ).SetFunct3(0b101).SetImplFunc( &divu ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("rem %rd, %rs1, %rs2"   ).SetFunct3(0b110).SetImplFunc( &rem ).InstEntry},
    {RevInstEntryBuilder<RevMInstDefaults>().SetMnemonic("remu %rd, %rs1, %rs20" ).SetFunct3(0b111).SetImplFunc( &remu ).InstEntry},
  };

public:
  /// RV32M: standard constructor
  RV32M( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV32M", Feature, RevMem, Output) {
    SetTable(std::move(RV32MTable));
  }
}; // end class RV32I

} // namespace SST::RevCPU

#endif
