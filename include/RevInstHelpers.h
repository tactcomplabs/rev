//
// _RevInstHelpers_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVINSTHELPERS_H_
#define _SST_REVCPU_REVINSTHELPERS_H_

#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

#include "RevFenv.h"
#include "RevInstTable.h"

namespace SST::RevCPU {

// Limits when converting from floating-point to integer
template<typename FP, typename INT>
inline constexpr FP fpmax = 0;
template<typename FP, typename INT>
inline constexpr FP fpmin = 0;
template<>
inline constexpr float fpmax<float, int32_t> = 0x1.fffffep+30f;
template<>
inline constexpr float fpmin<float, int32_t> = -0x1p+31f;
template<>
inline constexpr float fpmax<float, uint32_t> = 0x1.fffffep+31f;
template<>
inline constexpr float fpmin<float, uint32_t> = 0x0p+0f;
template<>
inline constexpr float fpmax<float, int64_t> = 0x1.fffffep+62f;
template<>
inline constexpr float fpmin<float, int64_t> = -0x1p+63f;
template<>
inline constexpr float fpmax<float, uint64_t> = 0x1.fffffep+63f;
template<>
inline constexpr float fpmin<float, uint64_t> = 0x0p+0f;
template<>
inline constexpr double fpmax<double, int32_t> = 0x1.fffffffcp+30;
template<>
inline constexpr double fpmin<double, int32_t> = -0x1p+31;
template<>
inline constexpr double fpmax<double, uint32_t> = 0x1.fffffffep+31;
template<>
inline constexpr double fpmin<double, uint32_t> = 0x0p+0;
template<>
inline constexpr double fpmax<double, int64_t> = 0x1.fffffffffffffp+62;
template<>
inline constexpr double fpmin<double, int64_t> = -0x1p+63;
template<>
inline constexpr double fpmax<double, uint64_t> = 0x1.fffffffffffffp+63;
template<>
inline constexpr double fpmin<double, uint64_t> = 0x0p+0;

/// General template for converting between Floating Point and Integer.
/// FP values outside the range of the target integer type are clipped
/// at the integer type's numerical limits, whether signed or unsigned.
template<typename FP, typename INT>
bool CvtFpToInt( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  // Read the FP register. Round to integer according to current rounding mode.
  FP fp = std::rint( R->GetFP<FP>( Inst.rs1 ) );

  // Convert to integer type
  INT res;
  if( std::isnan( fp ) || fp > fpmax<FP, INT> ) {
    std::feraiseexcept( FE_INVALID );
    res = std::numeric_limits<INT>::max();
  } else if( fp < fpmin<FP, INT> ) {
    std::feraiseexcept( FE_INVALID );
    res = std::numeric_limits<INT>::min();
  } else {
    res = static_cast<INT>( fp );
  }

  // Make final result signed so sign extension occurs when sizeof(INT) < XLEN
  R->SetX( Inst.rd, std::make_signed_t<INT>( res ) );

  R->AdvancePC( Inst );
  return true;
}

/// fclass: Return FP classification like the RISC-V fclass instruction
template<typename T>
unsigned fclass( T val ) {
  bool quietNaN;
  switch( std::fpclassify( val ) ) {
  case FP_INFINITE: return std::signbit( val ) ? 1u : 1u << 7;
  case FP_NAN:
    if constexpr( std::is_same_v<T, float> ) {
      uint32_t i32;
      memcpy( &i32, &val, sizeof( i32 ) );
      quietNaN = ( i32 & uint32_t{ 1 } << 22 ) != 0;
    } else {
      uint64_t i64;
      memcpy( &i64, &val, sizeof( i64 ) );
      quietNaN = ( i64 & uint64_t{ 1 } << 51 ) != 0;
    }
    return quietNaN ? 1u << 9 : 1u << 8;
  case FP_NORMAL: return std::signbit( val ) ? 1u << 1 : 1u << 6;
  case FP_SUBNORMAL: return std::signbit( val ) ? 1u << 2 : 1u << 5;
  case FP_ZERO: return std::signbit( val ) ? 1u << 3 : 1u << 4;
  default: return 0u;
  }
}

/// Load template
template<typename T>
bool load( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  if( sizeof( T ) < sizeof( int64_t ) && R->IsRV32 ) {
    static constexpr RevFlag flags =
      sizeof( T ) < sizeof( int32_t ) ? std::is_signed_v<T> ? RevFlag::F_SEXT32 : RevFlag::F_ZEXT32 : RevFlag::F_NONE;
    auto   rs1 = R->GetX<uint64_t>( Inst.rs1 );  // read once for tracer
    MemReq req{
      rs1 + Inst.ImmSignExt( 12 ),
      Inst.rd,
      RevRegClass::RegGPR,
      F->GetHartToExecID(),
      MemOp::MemOpREAD,
      true,
      R->GetMarkLoadComplete() };
    R->LSQueue->insert( req.LSQHashPair() );
    M->ReadVal(
      F->GetHartToExecID(), rs1 + Inst.ImmSignExt( 12 ), reinterpret_cast<T*>( &R->RV32[Inst.rd] ), std::move( req ), flags
    );
  } else {
    static constexpr RevFlag flags =
      sizeof( T ) < sizeof( int64_t ) ? std::is_signed_v<T> ? RevFlag::F_SEXT64 : RevFlag::F_ZEXT64 : RevFlag::F_NONE;
    auto   rs1 = R->GetX<uint64_t>( Inst.rs1 );
    MemReq req{
      rs1 + Inst.ImmSignExt( 12 ),
      Inst.rd,
      RevRegClass::RegGPR,
      F->GetHartToExecID(),
      MemOp::MemOpREAD,
      true,
      R->GetMarkLoadComplete() };
    R->LSQueue->insert( req.LSQHashPair() );
    M->ReadVal(
      F->GetHartToExecID(), rs1 + Inst.ImmSignExt( 12 ), reinterpret_cast<T*>( &R->RV64[Inst.rd] ), std::move( req ), flags
    );
  }

  // update the cost
  R->cost += M->RandCost( F->GetMinCost(), F->GetMaxCost() );
  R->AdvancePC( Inst );
  return true;
}

/// Store template
template<typename T>
bool store( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  M->Write( F->GetHartToExecID(), R->GetX<uint64_t>( Inst.rs1 ) + Inst.ImmSignExt( 12 ), R->GetX<T>( Inst.rs2 ) );
  R->AdvancePC( Inst );
  return true;
}

/// Floating-point load template
template<typename T>
bool fload( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  if( std::is_same_v<T, double> || F->HasD() ) {
    static constexpr RevFlag flags = sizeof( T ) < sizeof( double ) ? RevFlag::F_BOXNAN : RevFlag::F_NONE;
    uint64_t                 rs1   = R->GetX<uint64_t>( Inst.rs1 );
    MemReq                   req{
      rs1 + Inst.ImmSignExt( 12 ),
      Inst.rd,
      RevRegClass::RegFLOAT,
      F->GetHartToExecID(),
      MemOp::MemOpREAD,
      true,
      R->GetMarkLoadComplete() };
    R->LSQueue->insert( req.LSQHashPair() );
    M->ReadVal(
      F->GetHartToExecID(), rs1 + Inst.ImmSignExt( 12 ), reinterpret_cast<T*>( &R->DPF[Inst.rd] ), std::move( req ), flags
    );
  } else {
    uint64_t rs1 = R->GetX<uint64_t>( Inst.rs1 );
    MemReq   req{
      rs1 + Inst.ImmSignExt( 12 ),
      Inst.rd,
      RevRegClass::RegFLOAT,
      F->GetHartToExecID(),
      MemOp::MemOpREAD,
      true,
      R->GetMarkLoadComplete() };
    R->LSQueue->insert( req.LSQHashPair() );
    M->ReadVal( F->GetHartToExecID(), rs1 + Inst.ImmSignExt( 12 ), &R->SPF[Inst.rd], std::move( req ), RevFlag::F_NONE );
  }
  // update the cost
  R->cost += M->RandCost( F->GetMinCost(), F->GetMaxCost() );
  R->AdvancePC( Inst );
  return true;
}

/// Floating-point store template
template<typename T>
bool fstore( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  T val = R->GetFP<T, true>( Inst.rs2 );
  M->Write( F->GetHartToExecID(), R->GetX<uint64_t>( Inst.rs1 ) + Inst.ImmSignExt( 12 ), val );
  R->AdvancePC( Inst );
  return true;
}

/// Floating-point operation template
template<typename T, template<class> class OP>
bool foper( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  R->SetFP( Inst.rd, OP()( R->GetFP<T>( Inst.rs1 ), R->GetFP<T>( Inst.rs2 ) ) );
  R->AdvancePC( Inst );
  return true;
}

/// Floating-point minimum functor
template<typename = void>
struct FMin {
  template<typename T>
  auto operator()( T x, T y ) const {
    if( fclass( x ) == 1u << 8 || fclass( y ) == 1u << 8 )
      feraiseexcept( FE_INVALID );
    return std::fmin( x, y );
  }
};

/// Floating-point maximum functor
template<typename = void>
struct FMax {
  template<typename T>
  auto operator()( T x, T y ) const {
    if( fclass( x ) == 1u << 8 || fclass( y ) == 1u << 8 )
      feraiseexcept( FE_INVALID );
    return std::fmax( x, y );
  }
};

/// Floating-point conditional operation template
template<typename T, template<class> class OP>
bool fcondop( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  bool res = OP()( R->GetFP<T>( Inst.rs1 ), R->GetFP<T>( Inst.rs2 ) );
  R->SetX( Inst.rd, res );
  R->AdvancePC( Inst );
  return true;
}

/// Operand Kind (immediate or register)
enum class OpKind { Imm, Reg };

/// Integer arithmetic operator template
// The First parameter is the operator functor (such as std::plus)
// The second parameter is the operand kind (OpKind::Imm or OpKind::Reg)
// The third parameter is std::make_unsigned_t or std::make_signed_t (default)
// The optional fourth parameter indicates W mode (32-bit on XLEN == 64)
template<template<class> class OP, OpKind KIND, template<class> class SIGN = std::make_signed_t, bool W_MODE = false>
bool oper( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  if( !W_MODE && R->IsRV32 ) {
    using T = SIGN<int32_t>;
    T rs1   = R->GetX<T>( Inst.rs1 );
    T rs2   = KIND == OpKind::Imm ? T( Inst.ImmSignExt( 12 ) ) : R->GetX<T>( Inst.rs2 );
    T res   = OP()( rs1, rs2 );
    R->SetX( Inst.rd, res );
  } else {
    using T = SIGN<std::conditional_t<W_MODE, int32_t, int64_t>>;
    T rs1   = R->GetX<T>( Inst.rs1 );
    T rs2   = KIND == OpKind::Imm ? T( Inst.ImmSignExt( 12 ) ) : R->GetX<T>( Inst.rs2 );
    T res   = OP()( rs1, rs2 );
    // In W_MODE, cast the result to int32_t so that it's sign-extended
    R->SetX( Inst.rd, std::conditional_t<W_MODE, int32_t, T>( res ) );
  }
  R->AdvancePC( Inst );
  return true;
}

/// Left shift functor
template<typename = void>
struct ShiftLeft {
  template<typename T>
  constexpr T operator()( T val, unsigned shift ) const {
    return val << ( sizeof( T ) == 4 ? shift & 0x1f : shift & 0x3f );
  }
};

/// Right shift functor
template<typename = void>
struct ShiftRight {
  template<typename T>
  constexpr T operator()( T val, unsigned shift ) const {
    return val >> ( sizeof( T ) == 4 ? shift & 0x1f : shift & 0x3f );
  }
};

// Computes the UPPER half of multiplication, based on signedness
template<bool rs1_is_signed, bool rs2_is_signed>
bool uppermul( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  if( R->IsRV32 ) {
    uint32_t rs1 = R->GetX<uint32_t>( Inst.rs1 );
    uint32_t rs2 = R->GetX<uint32_t>( Inst.rs2 );
    uint32_t mul = static_cast<uint32_t>( rs1 * int64_t( rs2 ) >> 32 );
    if( rs1_is_signed && ( rs1 & ( uint32_t{ 1 } << 31 ) ) != 0 )
      mul -= rs2;
    if( rs2_is_signed && ( rs2 & ( uint32_t{ 1 } << 31 ) ) != 0 )
      mul -= rs1;
    R->SetX( Inst.rd, mul );
  } else {
    uint64_t rs1 = R->GetX<uint64_t>( Inst.rs1 );
    uint64_t rs2 = R->GetX<uint64_t>( Inst.rs2 );
    uint64_t mul = static_cast<uint64_t>( rs1 * __int128( rs2 ) >> 64 );
    if( rs1_is_signed && ( rs1 & ( uint64_t{ 1 } << 63 ) ) != 0 )
      mul -= rs2;
    if( rs2_is_signed && ( rs2 & ( uint64_t{ 1 } << 63 ) ) != 0 )
      mul -= rs1;
    R->SetX( Inst.rd, mul );
  }
  R->AdvancePC( Inst );
  return true;
}

enum class DivRem { Div, Rem };

/// Division/Remainder template
// The first parameter is DivRem::Div or DivRem::Rem
// The second parameter is std::make_signed_t or std::make_unsigned_t
// The optional third parameter indicates W mode (32-bit on XLEN == 64)
template<DivRem DIVREM, template<class> class SIGN, bool W_MODE = false>
bool divrem( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  if( !W_MODE && R->IsRV32 ) {
    using T = SIGN<int32_t>;
    T rs1   = R->GetX<T>( Inst.rs1 );
    T rs2   = R->GetX<T>( Inst.rs2 );
    T res;
    if constexpr( DIVREM == DivRem::Div ) {
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() && rs2 == -T{ 1 } ? rs1 : rs2 ? rs1 / rs2 : -T{ 1 };
    } else {
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() && rs2 == -T{ 1 } ? 0 : rs2 ? rs1 % rs2 : rs1;
    }
    R->SetX( Inst.rd, res );
  } else {
    using T = SIGN<std::conditional_t<W_MODE, int32_t, int64_t>>;
    T rs1   = R->GetX<T>( Inst.rs1 );
    T rs2   = R->GetX<T>( Inst.rs2 );
    T res;
    if constexpr( DIVREM == DivRem::Div ) {
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() && rs2 == -T{ 1 } ? rs1 : rs2 ? rs1 / rs2 : -T{ 1 };
    } else {
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() && rs2 == -T{ 1 } ? 0 : rs2 ? rs1 % rs2 : rs1;
    }
    // In W_MODE, cast the result to int32_t so that it's sign-extended
    R->SetX( Inst.rd, std::conditional_t<W_MODE, int32_t, T>( res ) );
  }
  R->AdvancePC( Inst );
  return true;
}

/// Conditional branch template
// The first template parameter is the comparison functor
// The second template parameter is std::make_signed_t or std::make_unsigned_t
template<template<class> class OP, template<class> class SIGN = std::make_unsigned_t>
bool bcond( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  bool cond;
  if( R->IsRV32 ) {
    cond = OP()( R->GetX<SIGN<int32_t>>( Inst.rs1 ), R->GetX<SIGN<int32_t>>( Inst.rs2 ) );
  } else {
    cond = OP()( R->GetX<SIGN<int64_t>>( Inst.rs1 ), R->GetX<SIGN<int64_t>>( Inst.rs2 ) );
  }
  if( cond ) {
    R->SetPC( R->GetPC() + Inst.ImmSignExt( 13 ) );
  } else {
    R->AdvancePC( Inst );
  }
  return true;
}

/// Negation function which flips sign bit, even of NaN
template<typename T>
inline auto negate( T x ) {
  return std::copysign( x, std::signbit( x ) ? T{ 1 } : T{ -1 } );
}

/// Rev FMA template which handles 0.0 * NAN and NAN * 0.0 correctly
// RISC-V requires INVALID exception when x * y is INVALID even when z = qNaN
template<typename T>
inline auto revFMA( T x, T y, T z ) {
  if( ( !y && std::isinf( x ) ) || ( !x && std::isinf( y ) ) ) {
    feraiseexcept( FE_INVALID );
  }
  return std::fma( x, y, z );
}

/// Fused Multiply+Add
template<typename T>
bool fmadd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  R->SetFP( Inst.rd, revFMA( R->GetFP<T>( Inst.rs1 ), R->GetFP<T>( Inst.rs2 ), R->GetFP<T>( Inst.rs3 ) ) );
  R->AdvancePC( Inst );
  return true;
}

/// Fused Multiply-Subtract
template<typename T>
bool fmsub( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  R->SetFP( Inst.rd, revFMA( R->GetFP<T>( Inst.rs1 ), R->GetFP<T>( Inst.rs2 ), negate( R->GetFP<T>( Inst.rs3 ) ) ) );
  R->AdvancePC( Inst );
  return true;
}

/// Fused Negated (Multiply-Subtract)
template<typename T>
bool fnmsub( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  R->SetFP( Inst.rd, revFMA( negate( R->GetFP<T>( Inst.rs1 ) ), R->GetFP<T>( Inst.rs2 ), R->GetFP<T>( Inst.rs3 ) ) );
  R->AdvancePC( Inst );
  return true;
}

/// Fused Negated (Multiply+Add)
template<typename T>
bool fnmadd( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
  R->SetFP( Inst.rd, negate( revFMA( R->GetFP<T>( Inst.rs1 ), R->GetFP<T>( Inst.rs2 ), R->GetFP<T>( Inst.rs3 ) ) ) );
  R->AdvancePC( Inst );
  return true;
}

}  // namespace SST::RevCPU

#endif
