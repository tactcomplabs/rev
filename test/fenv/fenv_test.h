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
    std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t> ix;
    std::memcpy( &ix, &x, sizeof( ix ) );
    if( !( ix & decltype( ix ){ 1 } << ( std::is_same_v<T, float> ? 22 : 51 ) ) ) {
      c = FP_SNAN;
    }
  }
  return c;
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

/// Converts FP to Integer
template<typename INT, typename T>
INT to_int( T x ) {
  using namespace std;

  if constexpr( std::is_same_v<T, float> ) {
    x = rintf( x );
  } else {
    x = rint( x );
  }

  if( isnan( x ) || x > fpmax<T, INT> ) {
    feraiseexcept( FE_INVALID );
    return numeric_limits<INT>::max();
  } else if( x < fpmin<T, INT> ) {
    feraiseexcept( FE_INVALID );
    return numeric_limits<INT>::min();
  } else {
    return static_cast<INT>( x );
  }
}

/// returns a type string
template<typename T>
inline constexpr char type[] = "(INVALID)";
template<>
inline constexpr char type<float>[] = "float";
template<>
inline constexpr char type<double>[] = "double";
template<>
inline constexpr char type<int32_t>[] = "int32_t";
template<>
inline constexpr char type<uint32_t>[] = "uint32_t";
template<>
inline constexpr char type<int64_t>[] = "int64_t";
template<>
inline constexpr char type<uint64_t>[] = "uint64_t";

/// Prints a value as a portable C++ exact constant
/// Handles +/- 0, +/- Inf, qNaN, sNaN
template<typename T>
const char* repr( T x ) {
  static char s[128];
  if constexpr( std::is_floating_point_v<T> ) {
    *s = 0;
    switch( float_class( x ) ) {
    case FP_NAN:
      strcat( s, "std::numeric_limits<" );
      strcat( s, type<T> );
      strcat( s, ">::quiet_NaN()" );
      break;
    case FP_SNAN:
      strcat( s, "std::numeric_limits<" );
      strcat( s, type<T> );
      strcat( s, ">::signaling_NaN()" );
      break;
    case FP_INFINITE:
      if( std::signbit( x ) )
        strcat( s, "-" );
      strcat( s, "std::numeric_limits<" );
      strcat( s, type<T> );
      strcat( s, ">::infinity()" );
      break;
    default: {
      static char fps[64];
      snprintf( fps, sizeof( fps ), "%a", (double) x );
      strcat( s, fps );
      strcat( s, std::is_same_v<T, float> ? "f" : "" );
    }
    }
  } else if constexpr( std::is_same_v<T, int32_t> ) {
    if( x == std::numeric_limits<T>::min() ) {
      snprintf( s, sizeof( s ), "-%" PRIi32 "-1", std::numeric_limits<T>::max() );
    } else {
      snprintf( s, sizeof( s ), "%" PRIi32, x );
    }
  } else if constexpr( std::is_same_v<T, int64_t> ) {
    static_assert( sizeof( long long ) == sizeof( int64_t ) );
    if( x == std::numeric_limits<T>::min() ) {
      snprintf( s, sizeof( s ), "-%" PRIi64 "ll-1", std::numeric_limits<T>::max() );
    } else {
      snprintf( s, sizeof( s ), "%" PRIi64 "ll", x );
    }
  } else if constexpr( std::is_same_v<T, uint32_t> ) {
    snprintf( s, sizeof( s ), "%" PRIu32 "u", x );
  } else if constexpr( std::is_same_v<T, uint64_t> ) {
    static_assert( sizeof( unsigned long long ) == sizeof( uint64_t ) );
    snprintf( s, sizeof( s ), "%" PRIu64 "llu", x );
  } else {
    static_assert( ( sizeof( T ), false ), "Error: Unknown data type\n" );
  }

  return s;
}

/// Formats a comma-separated list of values
template<typename... Ts>
const char* args_string( Ts... args ) {
  static char s[1024];
  const char* sep = "";
  s[0]            = 0;
  (void) ( ..., ( strcat( strcat( s, sep ), repr( args ) ), sep = ", " ) );
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

template<typename T, typename... Ts>
bool test_result( const char* test, const char* file_prefix, const char* test_src, T result, T result_expected, Ts... args ) {
  if constexpr( std::is_floating_point_v<T> ) {
    // Remove payloads from any NaNs
    for( T& x : { std::ref( result ), std::ref( result_expected ) } ) {
      switch( float_class( x ) ) {
      case FP_NAN: x = std::numeric_limits<T>::quiet_NaN(); break;
      case FP_SNAN: x = std::numeric_limits<T>::signaling_NaN(); break;
      }
    }
  }

  // Compare for exact bit representation equality
  if( memcmp( &result, &result_expected, sizeof( T ) ) ) {
    char s[1024];
    *s = 0;

    strcat( s, "\nResult error in fenv " );
    strcat( s, file_prefix );
    strcat( s, " Test " );
    strcat( s, test );
    strcat( s, ":\n" );
    strcat( s, test_src );
    strcat( s, "\n  ( " );
    strcat( s, args_string( args... ) );
    strcat( s, " )\n" );
    strcat( s, "Expected result: " );
    strcat( s, repr( result_expected ) );
    strcat( s, "\n" );
    strcat( s, "Actual   result: " );
    strcat( s, repr( result ) );
    strcat( s, "\n" );
    fenv_write( 2, s );
    return false;
  } else {
    return true;
  }
}

template<typename... Ts>
bool test_exceptions(
  const char* test,
  const char* file_prefix,
  const char* test_src,
  int         exceptions,
  int         exceptions_expected,
  bool        result_passed,
  Ts... args
) {
  char s[1024];
  *s = 0;
  if( ( exceptions ^ exceptions_expected ) & FE_ALL_EXCEPT ) {
    if( result_passed ) {
      strcat( s, "\nExceptions error in fenv " );
      strcat( s, file_prefix );
      strcat( s, " Test " );
      strcat( s, test );
      strcat( s, ":\n" );
      strcat( s, test_src );
      strcat( s, "\n  ( " );
      strcat( s, args_string( args... ) );
      strcat( s, " )\n" );
    }
    strcat( s, "Expected exceptions: " );
    strcat( s, exception_string( exceptions_expected ) );
    strcat( s, "\n" );
    strcat( s, "Actual   exceptions: " );
    strcat( s, exception_string( exceptions ) );
    strcat( s, "\n" );
    fenv_write( 2, s );
    return false;
  } else {
    if( result_passed ) {
      strcat( s, "\nfenv " );
      strcat( s, file_prefix );
      strcat( s, " Test " );
      strcat( s, test );
      strcat( s, " Passed\n" );
      fenv_write( 2, s );
    }
    return true;
  }
}

/// Rev FMA template which handles 0.0 * NAN and NAN * 0.0 correctly
// RISC-V requires INVALID exception when x * y is INVALID even when z = qNaN
template<typename T>
inline auto revFMA( T x, T y, T z ) {
  using namespace std;
  if( ( !y && isinf( x ) ) || ( !x && isinf( y ) ) ) {
    feraiseexcept( FE_INVALID );
  }
  if constexpr( is_same_v<T, float> ) {
    return fmaf( x, y, z );
  } else {
    return fma( x, y, z );
  }
}

#endif
