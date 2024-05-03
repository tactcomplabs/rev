#ifndef __FENV_TEST_H
#define __FENV_TEST_H

#include <cfenv>
#include <cinttypes>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

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

/// Prints a floating-point value as a portable C++ exact constant
/// Handles +/- 0, +/- Inf, qNaN, sNaN
template<typename T>
std::string float_string( T x ) {
  std::ostringstream s;
  const char*        type = std::is_same_v<T, float> ? "float" : "double";

  switch( float_class( x ) ) {
  case FP_NAN: s << "std::numeric_limits<" << type << ">::quiet_NaN()"; break;
  case FP_SNAN: s << "std::numeric_limits<" << type << ">::signaling_NaN()"; break;
  case FP_INFINITE:
    if( std::signbit( x ) )
      s << "-";
    s << "std::numeric_limits<" << type << ">::infinity()";
    break;
  default: s << std::hexfloat << x << ( std::is_same_v<T, float> ? "f" : "" );
  }
  return s.str();
}

/// Prints a comma-separated list of floating-point values
template<typename... Ts>
std::string args_string( Ts... args ) {
  std::ostringstream s;
  const char*        sep = "";
  ( ..., ( ( s << sep << float_string( args ), sep = ", " ) ) );
  return s.str();
}

/// Prints a logical-OR of exception names in an exception value
inline auto exception_string( int exceptions ) {
  std::ostringstream s;
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
        s << sep << e.second;
        sep = " | ";
      }
    }
  } else {
    s << "0";
  }
  return s.str();
}

template<typename T, typename... Ts>
bool test_result( unsigned test, std::string_view test_src, T result, T result_expected, Ts... args ) {
  // Remove payloads from any NaNs
  for( T& x : { std::ref( result ), std::ref( result_expected ) } ) {
    switch( float_class( x ) ) {
    case FP_NAN: x = std::numeric_limits<T>::quiet_NaN(); break;
    case FP_SNAN: x = std::numeric_limits<T>::signaling_NaN(); break;
    }
  }

  // Compare for exact bit representation equality
  if( memcmp( &result, &result_expected, sizeof( T ) ) ) {
    std::cerr << "\nError in fenv Test " << test << ":\n";
    std::cerr << test_src << "\n  ( " << args_string( args... ) << " )\n";
    std::cerr << "Expected result: " << float_string( result_expected ) << "\n";
    std::cerr << "Actual   result: " << float_string( result ) << "\n";
    return false;
  }
  return true;
}

template<typename... Ts>
bool test_exceptions(
  unsigned test, std::string_view test_src, int exceptions, int exceptions_expected, bool result_passed, Ts... args
) {
  if( ( exceptions ^ exceptions_expected ) & FE_ALL_EXCEPT ) {
    if( result_passed ) {
      std::cerr << "\nError in fenv Test " << test << ":\n";
      std::cerr << test_src << "\n  ( " << args_string( args... ) << " )\n";
    }
    std::cerr << "Expected exceptions: " << exception_string( exceptions_expected ) << "\n";
    std::cerr << "Actual   exceptions: " << exception_string( exceptions ) << "\n";
    return false;
  }
  return true;
}

#endif
