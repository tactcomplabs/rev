//
// _Zbb_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZBB_H_
#define _SST_REVCPU_ZBB_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zbb : public RevExt {

  template<typename = void>
  struct andn_func {
    template<typename T>
    auto operator()( T x, T y ) {
      return x & ~y;
    }
  };

  template<typename = void>
  struct orn_func {
    template<typename T>
    auto operator()( T x, T y ) {
      return x | ~y;
    }
  };

  template<typename = void>
  struct xnor_func {
    template<typename T>
    auto operator()( T x, T y ) {
      return ~( x ^ y );
    }
  };

  template<typename = void>
  struct max_func {
    template<typename T>
    auto operator()( T x, T y ) {
      return x > y ? x : y;
    }
  };

  template<typename = void>
  struct min_func {
    template<typename T>
    auto operator()( T x, T y ) {
      return x < y ? x : y;
    }
  };

  template<typename = void>
  struct orcb_func {
    template<typename T>
    auto operator()( T x ) {
      T res;
      for( size_t i = 0; i < sizeof( T ); ++i )
        reinterpret_cast<unsigned char*>( &res )[i] = reinterpret_cast<unsigned char*>( &x )[i] ? 0xFF : 0x00;
      return res;
    }
  };

  template<typename = void>
  struct clz_func {
    template<typename T>
    auto operator()( T x ) {
      if constexpr( sizeof( T ) == sizeof( long ) ) {
        return x ? __builtin_clzl( x ) : 64;
      } else {
        return x ? __builtin_clz( x ) : 32;
      }
    }
  };

  template<typename = void>
  struct ctz_func {
    template<typename T>
    auto operator()( T x ) {
      if constexpr( sizeof( T ) == sizeof( long ) ) {
        return x ? __builtin_ctzl( x ) : 64;
      } else {
        return x ? __builtin_ctz( x ) : 32;
      }
    }
  };

  template<typename = void>
  struct pop_func {
    template<typename T>
    auto operator()( T x ) {
      if constexpr( sizeof( T ) == sizeof( long ) ) {
        return __builtin_popcountl( x );
      } else {
        return __builtin_popcount( x );
      }
    }
  };

  template<typename = void>
  struct rev8_func {
    template<typename T>
    auto operator()( T x ) {
      T res;
      for( size_t i = 0; i < sizeof( T ); ++i )
        reinterpret_cast<unsigned char*>( &res )[i] = reinterpret_cast<unsigned char*>( &x )[sizeof( T ) - 1 - i];
      return res;
    }
  };

  template<typename = void>
  struct zexth_func {
    template<typename T>
    auto operator()( T x ) {
      return ZeroExt( x, 16 );
    }
  };

  template<typename = void>
  struct sexth_func {
    template<typename T>
    auto operator()( T x ) {
      return SignExt( x, 16 );
    }
  };

  template<typename = void>
  struct sextb_func {
    template<typename T>
    auto operator()( T x ) {
      return SignExt( x, 8 );
    }
  };

  template<typename = void>
  struct rol_func {
    template<typename T, typename U>
    auto operator()( T x, U y ) {
      int shamt = sizeof( T ) == sizeof( int32_t ) ? y & 0x1f : y & 0x3f;
      return x << shamt | x >> ( sizeof( T ) * 8 - shamt );
    }
  };

  template<typename = void>
  struct rolw_func {
    template<typename T, typename U>
    auto operator()( T x, U y ) {
      x          = ZeroExt( x, 32 );
      auto shamt = y & 0x1f;
      return SignExt( x << shamt | x >> ( 32 - shamt ), 32 );
    }
  };

  template<typename = void>
  struct ror_func {
    template<typename T, typename U>
    auto operator()( T x, U y ) {
      auto shamt = sizeof( T ) == sizeof( int32_t ) ? y & 0x1f : y & 0x3f;
      return x >> shamt | x << ( sizeof( T ) * 8 - shamt );
    }
  };

  template<typename = void>
  struct rori_func {
    template<typename T, typename U>
    auto operator()( T x, U y ) {
      x          = ZeroExt( x, 32 );
      auto shamt = y & 0x1f;
      return SignExt( x >> shamt | x << ( 32 - shamt ), 32 );
    }
  };

  template<typename = void>
  struct roriw_func {
    template<typename T, typename U>
    auto operator()( T x, U y ) {
      x          = ZeroExt( x, 32 );
      auto shamt = y & 0x1f;
      return SignExt( x >> shamt | x << ( 32 - shamt ), 32 );
    }
  };

  template<typename = void>
  struct rorw_func {
    template<typename T, typename U>
    auto operator()( T x, U y ) {
      auto shamt = sizeof( T ) == sizeof( int32_t ) ? y & 0x1f : y & 0x3f;
      return x >> shamt | x << ( sizeof( T ) * 8 - shamt );
    }
  };

  static constexpr auto& rol   = oper<rol_func, OpKind::Reg>;
  static constexpr auto& ror   = oper<ror_func, OpKind::Reg>;
  static constexpr auto& rolw  = oper<rolw_func, OpKind::Reg>;
  static constexpr auto& rorw  = oper<rorw_func, OpKind::Reg>;
  static constexpr auto& andn  = oper<andn_func, OpKind::Reg>;
  static constexpr auto& orn   = oper<orn_func, OpKind::Reg>;
  static constexpr auto& xnor  = oper<xnor_func, OpKind::Reg>;
  static constexpr auto& max   = oper<max_func, OpKind::Reg, std::make_signed_t>;
  static constexpr auto& maxu  = oper<max_func, OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& min   = oper<min_func, OpKind::Reg, std::make_signed_t>;
  static constexpr auto& minu  = oper<min_func, OpKind::Reg, std::make_unsigned_t>;

  static constexpr auto& clz   = operUnary<clz_func, false>;
  static constexpr auto& clzw  = operUnary<clz_func, true>;
  static constexpr auto& cpop  = operUnary<pop_func, false>;
  static constexpr auto& cpopw = operUnary<pop_func, true>;
  static constexpr auto& ctz   = operUnary<ctz_func, false>;
  static constexpr auto& ctzw  = operUnary<ctz_func, true>;
  static constexpr auto& rev8  = operUnary<rev8_func, false>;
  static constexpr auto& zexth = operUnary<zexth_func, false>;
  static constexpr auto& sexth = operUnary<sexth_func, false>;
  static constexpr auto& sextb = operUnary<sextb_func, false>;

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZbbInstDefaults : RevInstDefaults {
    RevZbbInstDefaults() {}
  };

  // clang-format off
  std::vector<RevInstEntry> ZbbTable = {
    RevZbbInstDefaults().SetMnemonic( "clz %rd, %rs"            ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( clz   ),
    RevZbbInstDefaults().SetMnemonic( "ctz %rd, %rs"            ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( ctz   ),
    RevZbbInstDefaults().SetMnemonic( "cpop %rd, %rs"           ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( cpop  ),
    RevZbbInstDefaults().SetMnemonic( "andn %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( andn  ),
    RevZbbInstDefaults().SetMnemonic( "orn %rd, %rs1, %rs2"     ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( orn   ),
    RevZbbInstDefaults().SetMnemonic( "xnor %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( xnor  ),
    RevZbbInstDefaults().SetMnemonic( "max %rd, %rs1, %rs2"     ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( max   ),
    RevZbbInstDefaults().SetMnemonic( "maxu %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( maxu  ),
    RevZbbInstDefaults().SetMnemonic( "min %rd, %rs1, %rs2"     ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( min   ),
    RevZbbInstDefaults().SetMnemonic( "minu %rd, %rs1, %rs2"    ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( minu  ),
    RevZbbInstDefaults().SetMnemonic( "orc.b %rd, %rs1"         ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( orcb  ),
    RevZbbInstDefaults().SetMnemonic( "rev8 %rd, %rs"           ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( rev8  ),
    RevZbbInstDefaults().SetMnemonic( "rol %rd, %rs1, %rs2"     ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( rol   ),
    RevZbbInstDefaults().SetMnemonic( "ror %rd, %rs1, %rs2"     ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( ror   ),
    RevZbbInstDefaults().SetMnemonic( "rori %rd, %rs1, %shamt"  ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( rori  ),
  };

  static std::vector<RevInstEntry> RV64ZbbTable() { return {
#if 0
    RevZbbInstDefaults().SetMnemonic( "clzw %rd, %rs"           ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( clzw   ),
    RevZbbInstDefaults().SetMnemonic( "cpopw %rd, %rs"          ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( cpopw  ),
    RevZbbInstDefaults().SetMnemonic( "roriw %rd, %rs1, %shamt" ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( roriw  ),
    RevZbbInstDefaults().SetMnemonic( "rolw %rd, %rs1, %shamt"  ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( rolw   ),
    RevZbbInstDefaults().SetMnemonic( "rorw %rd, %rs1, %shamt"  ).SetOpcode( 0b0110011 ).SetFunct3( 0b111 ).SetFunct2or7( 0b0100000 ).SetImplFunc( rorw   ),
#endif

  }; }

  // clang-format on

public:
  /// Zbb: standard constructor
  Zbb( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zbb", Feature, RevMem, Output ) {
    if( Feature->IsRV64() ) {
      auto RV64Table{ RV64ZbbTable() };
      RV64Table.insert( RV64Table.end(), std::move_iterator( ZbbTable.begin() ), std::move_iterator( ZbbTable.end() ) );
      SetTable( std::move( RV64Table ) );
    } else {
      SetTable( std::move( ZbbTable ) );
    }
  }

};  // end class Zbb

}  // namespace SST::RevCPU

#endif
