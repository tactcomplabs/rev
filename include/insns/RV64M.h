//
// _RV64M_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV64M_H_
#define _SST_REVCPU_RV64M_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

#include <limits>
#include <vector>

namespace SST::RevCPU {

class RV64M : public RevExt {
  // 32-bit Multiplication
  static constexpr auto& mulw  = oper<std::multiplies, OpKind::Reg, std::make_unsigned_t, true>;

  // 32-bit Division
  static constexpr auto& divw  = divrem<DivRem::Div, std::make_signed_t, true>;
  static constexpr auto& divuw = divrem<DivRem::Div, std::make_unsigned_t, true>;

  // 32-bit Remainder
  static constexpr auto& remw  = divrem<DivRem::Rem, std::make_signed_t, true>;
  static constexpr auto& remuw = divrem<DivRem::Rem, std::make_unsigned_t, true>;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV64M Instructions
  //
  // ----------------------------------------------------------------------
  struct Rev64MInstDefaults : RevInstDefaults {
    Rev64MInstDefaults() {
      SetOpcode( 0b0111011 );
      SetFunct2or7( 0b0000001 );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> RV64MTable = {
    Rev64MInstDefaults().SetMnemonic("mulw %rd, %rs1, %rs2" ).SetFunct3(0b000).SetImplFunc(mulw) ,
  };

  std::vector<RevInstEntry> RV64DivTable = {
    Rev64MInstDefaults().SetMnemonic("divw %rd, %rs1, %rs2" ).SetFunct3(0b100).SetImplFunc(divw) ,
    Rev64MInstDefaults().SetMnemonic("divuw %rd, %rs1, %rs2").SetFunct3(0b101).SetImplFunc(divuw),
    Rev64MInstDefaults().SetMnemonic("remw %rd, %rs1, %rs2" ).SetFunct3(0b110).SetImplFunc(remw) ,
    Rev64MInstDefaults().SetMnemonic("remuw %rd, %rs1, %rs2").SetFunct3(0b111).SetImplFunc(remuw),
  };
  // clang-format on

public:
  /// RV64M: standard constructor
  RV64M( const RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "RV64M", Feature, RevMem, Output ) {
    if( Feature->IsModeEnabled( RV_M ) ) {
      RV64MTable.insert( RV64MTable.end(), RV64DivTable.begin(), RV64DivTable.end() );
    }
    SetTable( std::move( RV64MTable ) );
  }
};  // end class RV32I

}  // namespace SST::RevCPU

#endif
