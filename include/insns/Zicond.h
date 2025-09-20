//
// _Zicond_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZICOND_H_
#define _SST_REVCPU_ZICOND_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zicond : public RevExt {

  template<template<typename> class CMP>
  static bool czero( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    if( R->IsRV32 ) {
      R->SetX( Inst.rd, CMP()( R->GetX<uint32_t>( Inst.rs2 ), 0 ) ? 0 : R->GetX<uint32_t>( Inst.rs1 ) );
    } else {
      R->SetX( Inst.rd, CMP()( R->GetX<uint64_t>( Inst.rs2 ), 0 ) ? 0 : R->GetX<uint64_t>( Inst.rs1 ) );
    }
    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& czero_eqz = czero<std::equal_to>;
  static constexpr auto& czero_nez = czero<std::not_equal_to>;

  // ----------------------------------------------------------------------
  //
  // RISC-V Zicond Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZicondInstDefaults : RevInstDefaults {
    RevZicondInstDefaults() {
      SetOpcode( 0b0110011 );
      SetFunct2or7( 0b0000111 );
    }
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Single-Precision Instructions
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // clang-format off
  std::vector<RevInstEntry> ZicondTable = {
    RevZicondInstDefaults().SetMnemonic( "czero.eqz %rd, %rs1, %rs2" ).SetFunct3( 0b101 ).SetImplFunc( czero_eqz ),
    RevZicondInstDefaults().SetMnemonic( "czero.nez %rd, %rs1, %rs2" ).SetFunct3( 0b111 ).SetImplFunc( czero_nez ),
  };

  // clang-format on

public:
  /// Zicond: standard constructor
  Zicond( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zicond", Feature, RevMem, Output ) {
    SetTable( std::move( ZicondTable ) );
  }
};  // end class Zicond

}  // namespace SST::RevCPU

#endif
