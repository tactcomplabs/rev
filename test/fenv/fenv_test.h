#ifndef __FENV_TEST_H
#define __FENV_TEST_H

#include <cfenv>
#include <cinttypes>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef __riscv
#include "syscalls.h"
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif

//#pragma STDC FENV_ACCESS ON

#define FP_SNAN 100  // A random value not returned by fpclassify()

template<typename T>
int float_class( T x ) {
  static_assert( std::numeric_limits<T>::is_iec559, "Environment does not support IEEE 754" );
  int c = std::fpclassify( x );
  if( c == FP_NAN ) {
    std::conditional_t<std::is_same<T, float>::value, uint32_t, uint64_t> ix;
    std::memcpy( &ix, &x, sizeof( ix ) );
    if( !( ix & decltype( ix ){ 1 } << ( std::is_same<T, float>::value ? 22 : 51 ) ) ) {
      c = FP_SNAN;
    }
  }
  return c;
}

template<typename T>
const char* fenv_hexfloat( T x ) {
  static char s[64];
  snprintf( s, sizeof( s ), "%a", (double) x );
  return s;
}

/// Prints a floating-point value as a portable C++ exact constant
/// Handles +/- 0, +/- Inf, qNaN, sNaN
template<typename T>
const char* float_string( T x ) {
  static char s[128];
  const char* type = std::is_same<T, float>::value ? "float" : "double";
  s[0]             = 0;

  switch( float_class( x ) ) {
  case FP_NAN:
    strcat( s, "std::numeric_limits<" );
    strcat( s, type );
    strcat( s, ">::quiet_NaN()" );
    break;
  case FP_SNAN:
    strcat( s, "std::numeric_limits<" );
    strcat( s, type );
    strcat( s, ">::signaling_NaN()" );
    break;
  case FP_INFINITE:
    if( std::signbit( x ) )
      strcat( s, "-" );
    strcat( s, "std::numeric_limits<" );
    strcat( s, type );
    strcat( s, ">::infinity()" );
    break;
  default: strcat( s, fenv_hexfloat( x ) ); strcat( s, std::is_same<T, float>::value ? "f" : "" );
  }
  return s;
}

/// Prints a comma-separated list of floating-point values
template<typename... Ts>
const char* args_string( Ts... args ) {
  static char s[256];
  const char* sep = "";
  s[0]            = 0;
  (void) ( ..., ( ( strcat( s, sep ), strcat( s, float_string( args ) ), sep = ", " ) ) );
  return s;
}

/// Prints a logical-OR of exception names in an exception value
inline const char* exception_string( int exceptions ) {
  static char s[128];
  s[0] = 0;
  if( exceptions ) {
    static constexpr std::pair<int, const char*> etable[] = {
      {FE_DIVBYZERO, "FE_DIVBYZERO"},
      {  FE_INEXACT,   "FE_INEXACT"},
      {  FE_INVALID,   "FE_INVALID"},
      { FE_OVERFLOW,  "FE_OVERFLOW"},
      {FE_UNDERFLOW, "FE_UNDERFLOW"},
    };
    const char* sep = "";
    for( auto& e : etable ) {
      if( exceptions & e.first ) {
        strcat( s, sep );
        strcat( s, e.second );
        sep = " | ";
      }
    }
  } else {
    strcat( s, "0" );
  }
  return s;
}

inline void fenv_write( int fd, const char* str ) {
  size_t len = 0;
  while( str[len] )
    len++;
#ifdef __riscv
  rev_write( fd, str, len );
#else
  write( fd, str, len );
#endif
}

template<typename T, typename... Ts>
bool test_result( const char* test, const char* test_src, T result, T result_expected, Ts... args ) {
  // Remove payloads from any NaNs
  for( T& x : { std::ref( result ), std::ref( result_expected ) } ) {
    switch( float_class( x ) ) {
    case FP_NAN: x = std::numeric_limits<T>::quiet_NaN(); break;
    case FP_SNAN: x = std::numeric_limits<T>::signaling_NaN(); break;
    }
  }

  // Compare for exact bit representation equality
  if( memcmp( &result, &result_expected, sizeof( T ) ) ) {
    fenv_write( 2, "\nError in fenv Test " );
    fenv_write( 2, test );
    fenv_write( 2, ":\n" );
    fenv_write( 2, test_src );
    fenv_write( 2, "\n  ( " );
    fenv_write( 2, args_string( args... ) );
    fenv_write( 2, " )\n" );
    fenv_write( 2, "Expected result: " );
    fenv_write( 2, float_string( result_expected ) );
    fenv_write( 2, "\n" );
    fenv_write( 2, "Actual   result: " );
    fenv_write( 2, float_string( result ) );
    fenv_write( 2, "\n" );
    return false;
  }
  return true;
}

template<typename... Ts>
bool test_exceptions(
  const char* test, std::string_view test_src, int exceptions, int exceptions_expected, bool result_passed, Ts... args
) {
  if( ( exceptions ^ exceptions_expected ) & FE_ALL_EXCEPT ) {
    if( result_passed ) {
      fenv_write( 2, "\nError in fenv Test " );
      fenv_write( 2, test );
      fenv_write( 2, ":\n" );
      fenv_write( 2, test_src.data() );
      fenv_write( 2, "\n  ( " );
      fenv_write( 2, args_string( args... ) );
      fenv_write( 2, " )\n" );
    }
    fenv_write( 2, "Expected exceptions: " );
    fenv_write( 2, exception_string( exceptions_expected ) );
    fenv_write( 2, "\n" );
    fenv_write( 2, "Actual   exceptions: " );
    fenv_write( 2, exception_string( exceptions ) );
    fenv_write( 2, "\n" );
    return false;
  }
  return true;
}

#endif
