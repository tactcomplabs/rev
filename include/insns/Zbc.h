//
// _Zbc_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZBC_H_
#define _SST_REVCPU_ZBC_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zbc : public RevExt {

  template<typename = void>
  struct clmul_func {
    template<typename T>
    auto operator()( T x, T y ) {
      if constexpr( sizeof( T ) == sizeof( uint64_t ) ) {
        __uint128_t res;
        asm volatile( "pclmullqlqdq" : "=r"( res ) : "r"( x ), "r"( y ) );
        return static_cast<uint64_t>( res );
      } else {
        uint64_t res;
        asm volatile( "pclmullqlqdq" : "=r"( res ) : "r"( x ), "r"( y ) );
        return static_cast<uint32_t>( res );
      }
    }
  };

  template<typename = void>
  struct clmulh_func {
    template<typename T>
    auto operator()( T x, T y ) {
      if constexpr( sizeof( T ) == sizeof( uint64_t ) ) {
        __uint128_t res;
        asm volatile( "pclmullqlqdq" : "=r"( res ) : "r"( x ), "r"( y ) );
        return static_cast<uint64_t>( res >> 64 );
      } else {
        uint64_t res;
        asm volatile( "pclmullqlqdq" : "=r"( res ) : "r"( x ), "r"( y ) );
        return static_cast<uint32_t>( res >> 32 );
      }
    }
  };

  template<typename T>
  T bit_rev( T x ) {
    if constexpr
  }

  template<typename = void>
  struct clmulr_func {
    template<typename T>
    auto operator()( T x, T y ) {
      return bit_rev( clmul_func()( bit_rev( x ), bit_rev( y ) ) );
    }
  };

  static constexpr auto& clmul  = oper<clmul_func>;
  static constexpr auto& clmulh = oper<clmulh_func>;
  static constexpr auto& clmulr = oper<clmulr_func>;

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZbcInstDefaults : RevInstDefaults {
    RevZbcInstDefaults() {}
  };

  // clang-format off
  std::vector<RevInstEntry> ZbcTable = {

    RevZbcInstDefaults().SetMnemonic( "clmul %rd, %rs1, %rs2"  ).SetOpcode( 0b0110011 ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010000 ).SetImplFunc( clmul  ),
    RevZbcInstDefaults().SetMnemonic( "clmulh %rd, %rs1, %rs2" ).SetOpcode( 0b0110011 ).SetFunct3( 0b100 ).SetFunct2or7( 0b0010000 ).SetImplFunc( clmulh ),
    RevZbcInstDefaults().SetMnemonic( "clmulr %rd, %rs1, %rs2" ).SetOpcode( 0b0110011 ).SetFunct3( 0b110 ).SetFunct2or7( 0b0010000 ).SetImplFunc( clmulr ),
#endif
  };
  // clang-format on

public:
  /// Zbc: standard constructor
  Zbc( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zbc", Feature, RevMem, Output ) {
    SetTable( std::move( ZbcTable ) );
  }

};  // end class Zbc

}  // namespace SST::RevCPU

#endif
