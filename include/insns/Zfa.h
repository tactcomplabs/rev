//
// _Zfa_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZFA_H_
#define _SST_REVCPU_ZFA_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zfa : public RevExt {

  // Round to integer without inexact exceptions
  template<typename T>
  static bool fround( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, std::nearbyint( R->GetFP<T>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  // Round to integer with inexact exceptions
  template<typename T>
  static bool froundnx( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetFP( Inst.rd, std::rint( R->GetFP<T>( Inst.rs1 ) ) );
    R->AdvancePC( Inst );
    return true;
  }

  /// Floating-point is less functor
  /// If any arguments are NaN, no exceptions are raised
  template<typename = void>
  struct isLess {
    template<typename T>
    auto operator()( T x, T y ) const {
      return std::isless( x, y );
    }
  };

  /// Floating-point is less or equal functor
  /// If any arguments are NaN, no exceptions are raised
  template<typename = void>
  struct isLessEqual {
    template<typename T>
    auto operator()( T x, T y ) const {
      return std::islessequal( x, y );
    }
  };

  /// Floating-point minimum/maximum
  /// If either argument is NaN, the result is NaN
  template<typename T, template<class> class MINMAX>
  static bool fminmaxm( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    T x   = R->GetFP<T>( Inst.rs1 );
    T y   = R->GetFP<T>( Inst.rs2 );
    T res = std::isunordered( x, y ) ? std::numeric_limits<T>::quiet_NaN() : MINMAX()( x, y );
    R->SetFP( Inst.rd, res );
    R->AdvancePC( Inst );
    return true;
  }

  // Move the high 32 bits of floating-point double to a XLEN==32 register
  static bool fmvhxd( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    double   fp = R->GetFP<double>( Inst.rs1 );
    uint64_t ifp;
    memcpy( &ifp, &fp, sizeof( ifp ) );
    static_assert( sizeof( fp ) == sizeof( ifp ) );
    R->SetX( Inst.rd, static_cast<uint32_t>( ifp >> 32 ) );
    R->AdvancePC( Inst );
    return true;
  }

  // Move a pair of XLEN==32 registers to a floating-point double
  static bool fmvpdx( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    uint64_t ifp = R->GetX<uint64_t>( Inst.rs1 ) | R->GetX<uint64_t>( Inst.rs2 ) << 32;
    double   fp;
    memcpy( &fp, &ifp, sizeof( fp ) );
    static_assert( sizeof( fp ) == sizeof( ifp ) );
    R->SetFP( Inst.rd, fp );
    R->AdvancePC( Inst );
    return true;
  }

  // Convert double precision floating point to 32-bit signed integer modulo 2^32
  // Always truncates (rounds toward 0) regardless of rounding mode
  // Raises the same exceptions as fcvt.w.d but the result is always modulo 2^32
  static bool fcvtmodwd( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {

    double   fp = R->GetFP<double>( Inst.rs1 );
    uint64_t ifp;
    static_assert( sizeof( fp ) == sizeof( ifp ) );
    memcpy( &ifp, &fp, sizeof( ifp ) );

    // clang-format off
    uint32_t exp{ static_cast<uint32_t>( ifp >> 52 & 0x7ff )           };
    uint64_t sig{ ( ifp & ~uint64_t{ 0 } >> 12 ) | uint64_t{ 1 } << 52 };
    bool      nv{ exp == 0x7ff                                         };
    bool      nx{                                                      };
    uint64_t res{ !nv && exp > 1011 && exp < 1139 ?
                      exp < 1075 ?
                          nx = sig << ( exp - 1011 ),
                          sig >> ( 1075 - exp )
                      :
                          sig << ( exp - 1075 )
                  :
                      0                                                };
    // clang-format on

    if( nv || exp > 1054 || res > ( ifp >> 63 ) + 0x7fffffff ) {
      std::feraiseexcept( FE_INVALID );
    } else if( nx ) {
      std::feraiseexcept( FE_INEXACT );
    }

    R->SetX( Inst.rd, SignExt( res, 32 ) );
    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& fminms    = fminmaxm<float, FMin>;
  static constexpr auto& fminmd    = fminmaxm<double, FMin>;

  static constexpr auto& fmaxms    = fminmaxm<float, FMax>;
  static constexpr auto& fmaxmd    = fminmaxm<double, FMax>;

  static constexpr auto& froundnxs = froundnx<float>;
  static constexpr auto& froundnxd = froundnx<double>;

  static constexpr auto& frounds   = fround<float>;
  static constexpr auto& froundd   = fround<double>;

  static constexpr auto& fleqs     = fcondop<float, isLessEqual>;
  static constexpr auto& fleqd     = fcondop<double, isLessEqual>;

  static constexpr auto& fltqs     = fcondop<float, isLess>;
  static constexpr auto& fltqd     = fcondop<double, isLess>;

  // ----------------------------------------------------------------------
  //
  // RISC-V Zfa Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZfaInstDefaults : RevInstDefaults {
    RevZfaInstDefaults() {
      SetOpcode( 0b1010011 );
      SetFunct3( 0b000 );
      SetFunct2or7( 0b1111000 );
      Setrs2fcvtOp( 0b00001 );
      SetrdClass( RevRegClass::RegFLOAT );
      Setrs1Class( RevRegClass::RegIMM );
      Setrs2Class( RevRegClass::RegIMM );
    }
  };

  // Macro to generate FLI entries
#define FLI( index, mnemonic, value )                                                \
  SetMnemonic( #mnemonic " %rd, " #value )                                           \
    .SetPredicate( []( uint32_t Inst ) { return DECODE_RS1( Inst ) == ( index ); } ) \
    .SetImplFunc( []( auto*, auto* R, auto*, auto& Inst ) {                          \
      R->SetFP( Inst.rd, value );                                                    \
      R->AdvancePC( Inst );                                                          \
      return true;                                                                   \
    } )

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Single-Precision Instructions
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef min
#undef inf
#undef nan
#define min std::numeric_limits<float>::min()
#define inf std::numeric_limits<float>::infinity()
#define nan std::numeric_limits<float>::quiet_NaN()

  // clang-format off
  std::vector<RevInstEntry> ZfaTable = {
    // Load Immediates
    RevZfaInstDefaults().FLI(  0, fli.s, -0x1p+0f  ),
    RevZfaInstDefaults().FLI(  1, fli.s, min       ),
    RevZfaInstDefaults().FLI(  2, fli.s, 0x1p-16f  ),
    RevZfaInstDefaults().FLI(  3, fli.s, 0x1p-15f  ),
    RevZfaInstDefaults().FLI(  4, fli.s, 0x1p-8f   ),
    RevZfaInstDefaults().FLI(  5, fli.s, 0x1p-7f   ),
    RevZfaInstDefaults().FLI(  6, fli.s, 0x1p-4f   ),
    RevZfaInstDefaults().FLI(  7, fli.s, 0x1p-3f   ),
    RevZfaInstDefaults().FLI(  8, fli.s, 0x1p-2f   ),
    RevZfaInstDefaults().FLI(  9, fli.s, 0x1.4p-2f ),
    RevZfaInstDefaults().FLI( 10, fli.s, 0x1.8p-2f ),
    RevZfaInstDefaults().FLI( 11, fli.s, 0x1.cp-2f ),
    RevZfaInstDefaults().FLI( 12, fli.s, 0x1p-1f   ),
    RevZfaInstDefaults().FLI( 13, fli.s, 0x1.4p-1f ),
    RevZfaInstDefaults().FLI( 14, fli.s, 0x1.8p-1f ),
    RevZfaInstDefaults().FLI( 15, fli.s, 0x1.cp-1f ),
    RevZfaInstDefaults().FLI( 16, fli.s, 0x1p+0f   ),
    RevZfaInstDefaults().FLI( 17, fli.s, 0x1.4p+0f ),
    RevZfaInstDefaults().FLI( 18, fli.s, 0x1.8p+0f ),
    RevZfaInstDefaults().FLI( 19, fli.s, 0x1.cp+0f ),
    RevZfaInstDefaults().FLI( 20, fli.s, 0x1p+1f   ),
    RevZfaInstDefaults().FLI( 21, fli.s, 0x1.4p+1f ),
    RevZfaInstDefaults().FLI( 22, fli.s, 0x1.8p+1f ),
    RevZfaInstDefaults().FLI( 23, fli.s, 0x1p+2f   ),
    RevZfaInstDefaults().FLI( 24, fli.s, 0x1p+3f   ),
    RevZfaInstDefaults().FLI( 25, fli.s, 0x1p+4f   ),
    RevZfaInstDefaults().FLI( 26, fli.s, 0x1p+7f   ),
    RevZfaInstDefaults().FLI( 27, fli.s, 0x1p+8f   ),
    RevZfaInstDefaults().FLI( 28, fli.s, 0x1p+15f  ),
    RevZfaInstDefaults().FLI( 29, fli.s, 0x1p+16f  ),
    RevZfaInstDefaults().FLI( 30, fli.s, inf       ),
    RevZfaInstDefaults().FLI( 31, fli.s, nan       ),

    // FP Minimum and Maximum with NaN returned if any argument is NaN
    RevZfaInstDefaults().SetMnemonic( "fminm.s %rd, %rs1, %rs2" ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ).SetRaiseFPE( true ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010100 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fminms ),
    RevZfaInstDefaults().SetMnemonic( "fmaxm.s %rd, %rs1, %rs2" ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ).SetRaiseFPE( true ).SetFunct3( 0b011 ).SetFunct2or7( 0b0010100 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fmaxms ),

    // FP Round To Integer with and without inexact exception
    RevZfaInstDefaults().SetMnemonic( "fround.s %rd, %rs1"   ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegUNKNOWN ).SetRaiseFPE( true ).SetFunct2or7( 0b0100000 ).Setrs2fcvtOp( 0b00100 ).SetImplFunc( frounds   ),
    RevZfaInstDefaults().SetMnemonic( "froundnx.s %rd, %rs1" ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegUNKNOWN ).SetRaiseFPE( true ).SetFunct2or7( 0b0100000 ).Setrs2fcvtOp( 0b00101 ).SetImplFunc( froundnxs ),

    // Quiet FP ordered comparison without raising exceptions on NaN inputs
    RevZfaInstDefaults().SetMnemonic( "fltq.s %rd, %rs1, %rs2" ).SetFunct3( 0b101 ).SetFunct2or7( 0b1010000 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fltqs ).SetrdClass( RevRegClass::RegGPR ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ),
    RevZfaInstDefaults().SetMnemonic( "fleq.s %rd, %rs1, %rs2" ).SetFunct3( 0b100 ).SetFunct2or7( 0b1010000 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fleqs ).SetrdClass( RevRegClass::RegGPR ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ),

  };

  // clang-format on

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Double-Precision Instructions
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct RevZfaDInstDefaults : RevZfaInstDefaults {
    RevZfaDInstDefaults() { SetFunct2or7( 0b1111001 ); }
  };

#undef min
#undef inf
#undef nan
#define min std::numeric_limits<double>::min()
#define inf std::numeric_limits<double>::infinity()
#define nan std::numeric_limits<double>::quiet_NaN()

  // clang-format off
  static std::vector<RevInstEntry> ZfaTableD() { return {
      // Load Immediates
       RevZfaDInstDefaults().FLI(  0, fli.d, -0x1p+0  ),
       RevZfaDInstDefaults().FLI(  1, fli.d, min      ),
       RevZfaDInstDefaults().FLI(  2, fli.d, 0x1p-16  ),
       RevZfaDInstDefaults().FLI(  3, fli.d, 0x1p-15  ),
       RevZfaDInstDefaults().FLI(  4, fli.d, 0x1p-8   ),
       RevZfaDInstDefaults().FLI(  5, fli.d, 0x1p-7   ),
       RevZfaDInstDefaults().FLI(  6, fli.d, 0x1p-4   ),
       RevZfaDInstDefaults().FLI(  7, fli.d, 0x1p-3   ),
       RevZfaDInstDefaults().FLI(  8, fli.d, 0x1p-2   ),
       RevZfaDInstDefaults().FLI(  9, fli.d, 0x1.4p-2 ),
       RevZfaDInstDefaults().FLI( 10, fli.d, 0x1.8p-2 ),
       RevZfaDInstDefaults().FLI( 11, fli.d, 0x1.cp-2 ),
       RevZfaDInstDefaults().FLI( 12, fli.d, 0x1p-1   ),
       RevZfaDInstDefaults().FLI( 13, fli.d, 0x1.4p-1 ),
       RevZfaDInstDefaults().FLI( 14, fli.d, 0x1.8p-1 ),
       RevZfaDInstDefaults().FLI( 15, fli.d, 0x1.cp-1 ),
       RevZfaDInstDefaults().FLI( 16, fli.d, 0x1p+0   ),
       RevZfaDInstDefaults().FLI( 17, fli.d, 0x1.4p+0 ),
       RevZfaDInstDefaults().FLI( 18, fli.d, 0x1.8p+0 ),
       RevZfaDInstDefaults().FLI( 19, fli.d, 0x1.cp+0 ),
       RevZfaDInstDefaults().FLI( 20, fli.d, 0x1p+1   ),
       RevZfaDInstDefaults().FLI( 21, fli.d, 0x1.4p+1 ),
       RevZfaDInstDefaults().FLI( 22, fli.d, 0x1.8p+1 ),
       RevZfaDInstDefaults().FLI( 23, fli.d, 0x1p+2   ),
       RevZfaDInstDefaults().FLI( 24, fli.d, 0x1p+3   ),
       RevZfaDInstDefaults().FLI( 25, fli.d, 0x1p+4   ),
       RevZfaDInstDefaults().FLI( 26, fli.d, 0x1p+7   ),
       RevZfaDInstDefaults().FLI( 27, fli.d, 0x1p+8   ),
       RevZfaDInstDefaults().FLI( 28, fli.d, 0x1p+15  ),
       RevZfaDInstDefaults().FLI( 29, fli.d, 0x1p+16  ),
       RevZfaDInstDefaults().FLI( 30, fli.d, inf      ),
       RevZfaDInstDefaults().FLI( 31, fli.d, nan      ),

      // FP Minimum and Maximum with NaN returned if any argument is NaN
      RevZfaDInstDefaults().SetMnemonic( "fminm.d %rd, %rs1, %rs2" ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ).SetRaiseFPE( true ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010101 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fminmd ),
      RevZfaDInstDefaults().SetMnemonic( "fmaxm.d %rd, %rs1, %rs2" ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ).SetRaiseFPE( true ).SetFunct3( 0b011 ).SetFunct2or7( 0b0010101 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fmaxmd ),

      // FP Round To Integer with and without inexact exception
      RevZfaDInstDefaults().SetMnemonic( "fround.d %rd, %rs1"   ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegUNKNOWN ).SetRaiseFPE( true ).SetFunct2or7( 0b0100001 ).Setrs2fcvtOp( 0b00100 ).SetImplFunc( froundd   ),
      RevZfaDInstDefaults().SetMnemonic( "froundnx.d %rd, %rs1" ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegUNKNOWN ).SetRaiseFPE( true ).SetFunct2or7( 0b0100001 ).Setrs2fcvtOp( 0b00101 ).SetImplFunc( froundnxd ),

      // Quiet FP ordered comparison without raising exceptions on NaN inputs
      RevZfaDInstDefaults().SetMnemonic( "fltq.d %rd, %rs1, %rs2" ).SetFunct3( 0b101 ).SetFunct2or7( 0b1010001 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fltqd ).SetrdClass( RevRegClass::RegGPR ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ),
      RevZfaDInstDefaults().SetMnemonic( "fleq.d %rd, %rs1, %rs2" ).SetFunct3( 0b100 ).SetFunct2or7( 0b1010001 ).Setrs2fcvtOp( 0b00000 ).SetImplFunc( fleqd ).SetrdClass( RevRegClass::RegGPR ).Setrs1Class( RevRegClass::RegFLOAT ).Setrs2Class( RevRegClass::RegFLOAT ),

      // Modular conversion to integer
      RevZfaDInstDefaults().SetMnemonic( "fcvtmod.w.d %rd, %rs1, rtz" ).SetFunct2or7( 0b1100001 ).SetImplFunc( fcvtmodwd ).SetrdClass( RevRegClass::RegGPR ).Setrs2fcvtOp( 0b01000 ).SetFunct3( 0b001 ).Setrs1Class( RevRegClass::RegFLOAT ).SetRaiseFPE( true ),

    }; }

  // clang-format on

#undef min
#undef inf
#undef nan

public:
  /// Zfa: standard constructor
  Zfa( const RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zfa", Feature, RevMem, Output ) {
    if( Feature->HasD() ) {
      if( !Feature->IsRV64() ) {
        // clang-format off
        ZfaTable.push_back(
            RevZfaInstDefaults().SetMnemonic( "fmvh.x.d %rd, %rs1"       ).SetFunct2or7( 0b1110001 ).SetImplFunc( fmvhxd ).SetrdClass( RevRegClass::RegGPR ).Setrs1Class( RevRegClass::RegFLOAT )
        );
        ZfaTable.push_back(
            RevZfaInstDefaults().SetMnemonic( "fmvp.d.x %rd, %rs1, %rs2" ).SetFunct2or7( 0b1011001 ).SetImplFunc( fmvpdx ).Setrs1Class( RevRegClass::RegGPR ).Setrs2Class( RevRegClass::RegGPR )
        );
        // clang-format on
      }
      auto TableD{ ZfaTableD() };
      ZfaTable.insert( ZfaTable.end(), std::move_iterator( TableD.begin() ), std::move_iterator( TableD.end() ) );
    }
    SetTable( std::move( ZfaTable ) );
  }

};  // end class Zfa

}  // namespace SST::RevCPU

#endif
