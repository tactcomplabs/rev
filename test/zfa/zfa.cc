/*
 * zfa.cc
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

// #pragma STDC FENV_ACCESS ON

#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

#define FLI_TEST( type, letter, value )                             \
  do {                                                              \
    type expected = value;                                          \
    type result;                                                    \
    asm volatile( "fli." #letter " %0, " #value : "=f"( result ) ); \
    if( memcmp( &result, &expected, sizeof( result ) ) )            \
      asm( ".word 0" );                                             \
  } while( 0 )

#define RUN_FLI_TESTS( type, letter )                  \
  do {                                                 \
    type min = std::numeric_limits<type>::min();       \
    type inf = std::numeric_limits<type>::infinity();  \
    type nan = std::numeric_limits<type>::quiet_NaN(); \
    FLI_TEST( type, letter, -0x1p+0 );                 \
    FLI_TEST( type, letter, min );                     \
    FLI_TEST( type, letter, 0x1p-16 );                 \
    FLI_TEST( type, letter, 0x1p-15 );                 \
    FLI_TEST( type, letter, 0x1p-8 );                  \
    FLI_TEST( type, letter, 0x1p-7 );                  \
    FLI_TEST( type, letter, 0x1p-4 );                  \
    FLI_TEST( type, letter, 0x1p-3 );                  \
    FLI_TEST( type, letter, 0x1p-2 );                  \
    FLI_TEST( type, letter, 0x1.4p-2 );                \
    FLI_TEST( type, letter, 0x1.8p-2 );                \
    FLI_TEST( type, letter, 0x1.cp-2 );                \
    FLI_TEST( type, letter, 0x1p-1 );                  \
    FLI_TEST( type, letter, 0x1.4p-1 );                \
    FLI_TEST( type, letter, 0x1.8p-1 );                \
    FLI_TEST( type, letter, 0x1.cp-1 );                \
    FLI_TEST( type, letter, 0x1p+0 );                  \
    FLI_TEST( type, letter, 0x1.4p+0 );                \
    FLI_TEST( type, letter, 0x1.8p+0 );                \
    FLI_TEST( type, letter, 0x1.cp+0 );                \
    FLI_TEST( type, letter, 0x1p+1 );                  \
    FLI_TEST( type, letter, 0x1.4p+1 );                \
    FLI_TEST( type, letter, 0x1.8p+1 );                \
    FLI_TEST( type, letter, 0x1p+2 );                  \
    FLI_TEST( type, letter, 0x1p+3 );                  \
    FLI_TEST( type, letter, 0x1p+4 );                  \
    FLI_TEST( type, letter, 0x1p+7 );                  \
    FLI_TEST( type, letter, 0x1p+8 );                  \
    FLI_TEST( type, letter, 0x1p+15 );                 \
    FLI_TEST( type, letter, 0x1p+16 );                 \
    FLI_TEST( type, letter, inf );                     \
    FLI_TEST( type, letter, nan );                     \
  } while( 0 )

#define FMINMAXM_TEST( type, inst, letter, val1, val2 )                                           \
  do {                                                                                            \
    type arg1 = val1, arg2 = val2;                                                                \
    type result;                                                                                  \
    asm volatile( #inst "." #letter " %0, %1, %2 " : "=f"( result ) : "f"( arg1 ), "f"( arg2 ) ); \
    if( std::isnan( arg1 ) || std::isnan( arg2 ) ) {                                              \
      if( !std::isnan( result ) )                                                                 \
        asm( ".word 0" );                                                                         \
    } else {                                                                                      \
      if( std::isnan( result ) )                                                                  \
        asm( ".word 0" );                                                                         \
    }                                                                                             \
  } while( 0 )

#define RUN_FMINMAXM_TESTS( type, inst, letter )                                                                         \
  do {                                                                                                                   \
    FMINMAXM_TEST( type, inst, letter, std::numeric_limits<type>::quiet_NaN(), 0.0f );                                   \
    FMINMAXM_TEST( type, inst, letter, 0.0f, std::numeric_limits<type>::quiet_NaN() );                                   \
    FMINMAXM_TEST( type, inst, letter, std::numeric_limits<type>::quiet_NaN(), std::numeric_limits<type>::quiet_NaN() ); \
    FMINMAXM_TEST( type, inst, letter, std::numeric_limits<type>::infinity(), 0.0f );                                    \
  } while( 0 )

#define ROUND_TEST( type, inst, letter, exceptions, val )                      \
  do {                                                                         \
    type result;                                                               \
    type arg = val;                                                            \
    std::feclearexcept( FE_ALL_EXCEPT );                                       \
    asm volatile( #inst "." #letter " %0, %1" : "=f"( result ) : "f"( arg ) ); \
    if( std::fetestexcept( FE_ALL_EXCEPT ) != exceptions )                     \
      asm( ".word 0" );                                                        \
  } while( 0 )

#define RUN_ROUND_TESTS( type, letter )                                                           \
  do {                                                                                            \
    ROUND_TEST( type, froundnx, letter, FE_INVALID, std::numeric_limits<type>::signaling_NaN() ); \
    ROUND_TEST( type, froundnx, letter, FE_INEXACT, 1.5f );                                       \
    ROUND_TEST( type, froundnx, letter, 0, 1.0f );                                                \
    ROUND_TEST( type, fround, letter, FE_INVALID, std::numeric_limits<type>::signaling_NaN() );   \
    ROUND_TEST( type, fround, letter, 0, 1.5f );                                                  \
    ROUND_TEST( type, fround, letter, 0, 1.0f );                                                  \
  } while( 0 )

#define QUIET_COMPARE_TEST( type, inst, letter, val1, val2 )                                     \
  do {                                                                                           \
    size_t result;                                                                               \
    std::feclearexcept( FE_ALL_EXCEPT );                                                         \
    asm volatile( #inst "." #letter " %0, %1, %2" : "=r"( result ) : "f"( val1 ), "f"( val2 ) ); \
    if( std::fetestexcept( FE_INVALID ) )                                                        \
      asm( ".word 0" );                                                                          \
  } while( 0 )

#define RUN_QUIET_COMPARE_TESTS( type, inst, letter )                                                                         \
  do {                                                                                                                        \
    QUIET_COMPARE_TEST( type, inst, letter, std::numeric_limits<type>::signaling_NaN(), type{ 0 } );                          \
    QUIET_COMPARE_TEST( type, inst, letter, std::numeric_limits<type>::quiet_NaN(), type{ 0 } );                              \
    QUIET_COMPARE_TEST( type, inst, letter, type{ 0 }, std::numeric_limits<type>::signaling_NaN() );                          \
    QUIET_COMPARE_TEST( type, inst, letter, type{ 0 }, std::numeric_limits<type>::quiet_NaN() );                              \
    QUIET_COMPARE_TEST( type, inst, letter, std::numeric_limits<type>::quiet_NaN(), std::numeric_limits<type>::quiet_NaN() ); \
    QUIET_COMPARE_TEST(                                                                                                       \
      type, inst, letter, std::numeric_limits<type>::signaling_NaN(), std::numeric_limits<type>::signaling_NaN()              \
    );                                                                                                                        \
  } while( 0 )

#ifdef __riscv_zfa

#if __riscv_flen >= 64

void fcvtmod_w_d_test( double value, intptr_t expected, int exceptions ) {
  intptr_t result;
  std::feclearexcept( FE_ALL_EXCEPT );
  asm volatile( "fcvtmod.w.d %0, %1, rtz" : "=r"( result ) : "f"( value ) );
  if( result != expected )
    asm( ".word 0" );
  if( fetestexcept( FE_ALL_EXCEPT ) != exceptions )
    asm( ".word 0" );
}

void run_fcvtmod_w_d_tests() {
  fcvtmod_w_d_test( std::numeric_limits<double>::signaling_NaN(), 0, FE_INVALID );
  fcvtmod_w_d_test( std::numeric_limits<double>::quiet_NaN(), 0, FE_INVALID );
  fcvtmod_w_d_test( std::numeric_limits<double>::infinity(), 0, FE_INVALID );
  fcvtmod_w_d_test( 0x1p+32, 0, FE_INVALID );
  fcvtmod_w_d_test( 0x1p+31, -2147483647 - 1, FE_INVALID );
  fcvtmod_w_d_test( -0x1p+31, -2147483647 - 1, 0 );
  fcvtmod_w_d_test( -0x1.00000002p+31, -2147483647, FE_INVALID );
  fcvtmod_w_d_test( -0x1.0000000000001p+31, -2147483647 - 1, FE_INEXACT );
  fcvtmod_w_d_test( 1024.5, 1024, FE_INEXACT );
}

#endif

#endif

int main( int argc, char** argv ) {

#ifdef __riscv_zfa

#if __riscv_flen >= 32

  RUN_FLI_TESTS( float, s );

  RUN_FMINMAXM_TESTS( float, fminm, s );
  RUN_FMINMAXM_TESTS( float, fmaxm, s );

  RUN_QUIET_COMPARE_TESTS( float, fltq, s );
  RUN_QUIET_COMPARE_TESTS( float, fleq, s );

  RUN_ROUND_TESTS( float, s );

#endif

#if __riscv_flen >= 64

  RUN_FLI_TESTS( double, d );

  RUN_FMINMAXM_TESTS( double, fminm, d );
  RUN_FMINMAXM_TESTS( double, fmaxm, d );

  RUN_QUIET_COMPARE_TESTS( double, fltq, d );
  RUN_QUIET_COMPARE_TESTS( double, fleq, d );

  RUN_ROUND_TESTS( double, d );

  run_fcvtmod_w_d_tests();

#endif

#endif

  return 0;
}
