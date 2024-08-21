//
// _Zba_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZBA_H_
#define _SST_REVCPU_ZBA_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zba : public RevExt {

  template<int SHIFT, bool W_MODE>
  static bool shadd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    if constexpr( W_MODE ) {
      R->SetX( Inst.rd, ( uint64_t{ R->GetX<uint32_t>( Inst.rs1 ) } << SHIFT ) + R->GetX<uint64_t>( Inst.rs2 ) );
    } else if( F->IsRV64() ) {
      R->SetX( Inst.rd, ( R->GetX<uint64_t>( Inst.rs1 ) << SHIFT ) + R->GetX<uint64_t>( Inst.rs2 ) );
    } else {
      R->SetX( Inst.rd, ( R->GetX<uint32_t>( Inst.rs1 ) << SHIFT ) + R->GetX<uint32_t>( Inst.rs2 ) );
    }
    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& adduw    = shadd<0, true>;
  static constexpr auto& sh1add   = shadd<1, false>;
  static constexpr auto& sh1adduw = shadd<1, true>;
  static constexpr auto& sh2add   = shadd<2, false>;
  static constexpr auto& sh2adduw = shadd<2, true>;
  static constexpr auto& sh3add   = shadd<3, false>;
  static constexpr auto& sh3adduw = shadd<3, true>;

  static bool slliuw( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetX( Inst.rd, uint64_t{ R->GetX<uint32_t>( Inst.rs1 ) } << ( Inst.imm & 0x3f ) );
    R->AdvancePC( Inst );
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZbaInstDefaults : RevInstDefaults {
    RevZbaInstDefaults() {}
  };

  // clang-format off
  std::vector<RevInstEntry> ZbaTable = {
    RevZbaInstDefaults().SetMnemonic( "sh1add %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010000 ).SetImplFunc( sh1add   ),
    RevZbaInstDefaults().SetMnemonic( "sh2add %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b100 ).SetFunct2or7( 0b0010000 ).SetImplFunc( sh2add   ),
    RevZbaInstDefaults().SetMnemonic( "sh3add %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b110 ).SetFunct2or7( 0b0010000 ).SetImplFunc( sh3add   ),
  };

  static std::vector<RevInstEntry> RV64ZbaTable() { return {
    RevZbaInstDefaults().SetMnemonic( "add.uw %rd, %rs1, %rs2"    ).SetOpcode( 0b0111011 ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000100 ).SetImplFunc( adduw    ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS2( Inst ) != 0; } ),
    RevZbaInstDefaults().SetMnemonic( "zext.w %rd, %rs"           ).SetOpcode( 0b0111011 ).SetFunct3( 0b000 ).SetFunct2or7( 0b0000100 ).SetImplFunc( adduw    ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS2( Inst ) == 0; } ),
    RevZbaInstDefaults().SetMnemonic( "sh1add.uw %rd, %rs1, %rs2" ).SetOpcode( 0b0111011 ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010000 ).SetImplFunc( sh1adduw ),
    RevZbaInstDefaults().SetMnemonic( "sh2add.uw %rd, %rs1, %rs2" ).SetOpcode( 0b0111011 ).SetFunct3( 0b100 ).SetFunct2or7( 0b0010000 ).SetImplFunc( sh2adduw ),
    RevZbaInstDefaults().SetMnemonic( "sh3add.uw %rd, %rs1, %rs2" ).SetOpcode( 0b0111011 ).SetFunct3( 0b110 ).SetFunct2or7( 0b0100000 ).SetImplFunc( sh3adduw ),
    RevZbaInstDefaults().SetMnemonic( "slli.uw %rd, %rs1, $imm"   ).SetOpcode( 0b0011011 ).SetFunct3( 0b001 ).SetFunct2or7( 0b000010  ).SetImplFunc( slliuw   ).SetFormat( RVTypeI ),
  }; }

  // clang-format on

public:
  /// Zba: standard constructor
  Zba( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zba", Feature, RevMem, Output ) {
    if( Feature->IsRV64() ) {
      auto RV64Table{ RV64ZbaTable() };
      RV64Table.insert( RV64Table.end(), std::move_iterator( ZbaTable.begin() ), std::move_iterator( ZbaTable.end() ) );
      SetTable( std::move( RV64Table ) );
    } else {
      SetTable( std::move( ZbaTable ) );
    }
  }

};  // end class Zba

}  // namespace SST::RevCPU

#endif
