/*
 * fenv_gentests.cc
 *
 * RISC-V ISA: RV32F, RV32D, RV64F, RV64D
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include "fenv_test.h"

template< typename T >
auto& print_float( T x ) {
  const char* type = std::is_same_v< T, float > ? "float" : "double";
  if( std::isnan( x ) ) {
    std::cout << "std::numeric_limits<" << type << ">::";
    std::conditional_t< std::is_same_v< T, float >, uint32_t, uint64_t > ix;
    std::memcpy( &ix, &x, sizeof( ix ) );
    if( ix & decltype( ix ){ 1 } << ( std::is_same_v< T, float > ? 22 : 51 ) ) {
      return std::cout << "quiet_NaN()";
    } else {
      return std::cout << "signaling_NaN()";
    }
  } else if( std::isinf( x ) ) {
    return std::cout << ( x < 0 ? "-" : "" ) << "std::numeric_limits<" << type
                     << ">::infinity()";
  } else {
    return std::cout << x << ( std::is_same_v< T, float > ? "f" : "" );
  }
}

static unsigned testno;
static int      rounding;

template< typename T, typename... Ts >
void generate_test(
  const std::pair< T ( * )( Ts... ), std::string_view >& oper_pair,
  Ts... ops ) {
  auto& [func, func_src] = oper_pair;
  fenv_t fenv;
  std::fesetround( rounding );
  std::feholdexcept( &fenv );

  T   result  = func( ops... );
  int excepts = std::fetestexcept( FE_ALL_EXCEPT );

  std::cout << "  []{\n";
  std::cout << "    // Test " << ++testno << "\n";
  std::cout << "    using namespace std;\n";
  std::cout << "    auto func = " << func_src << ";\n";
  std::cout << "    auto func_src = \"" << func_src << "\";\n";
  std::cout << "    fenv_t fenv;\n";

  switch( rounding ) {
  case FE_TONEAREST: std::cout << "    fesetround( FE_TONEAREST );\n"; break;
  case FE_TOWARDZERO: std::cout << "    fesetround( FE_TOWARDZERO );\n"; break;
  case FE_UPWARD: std::cout << "    fesetround( FE_UPWARD );\n"; break;
  case FE_DOWNWARD: std::cout << "    fesetround( FE_DOWNWARD );\n"; break;
  }

  std::cout << "    feholdexcept( &env );\n";
  std::cout << "    auto result = func( " << args_string( ops... ) << " );\n";
  std::cout << "    int exceptions = fetestexcept( FE_ALL_EXCEPT );\n";
  std::cout << "    auto result_expected = " << float_string( result ) << ";\n";
  std::cout << "    int exceptions_expected = " << exception_string( excepts )
            << ";\n";
  std::cout << "    bool result_passed = test_result( " << testno
            << ", func_src, result, result_expected, " << args_string( ops... )
            << " );\n";
  std::cout << "    bool exceptions_passed = test_exceptions( " << testno
            << ", func_src, exceptions, exceptions_expected, result_passed, "
            << args_string( ops... ) << ");\n";
  std::cout << "    failures += !(result_passed && exceptions_passed);\n";
  std::cout << "    fesetenv( &fenv );\n";
  std::cout << "  },\n";
  std::fesetenv( &fenv );
}

template< typename FP, typename INT >
constexpr FP special_values[] = {
  0.0f,
  -0.0f,
  1.0f,
  -1.0f,
  1.5f,
  -1.5f,
  (FP) 3.1,
  (FP) -3.1,
  std::numeric_limits< FP >::quiet_NaN(),
  std::numeric_limits< FP >::signaling_NaN(),
  std::numeric_limits< FP >::infinity(),
  -std::numeric_limits< FP >::infinity(),
  std::numeric_limits< FP >::denorm_min(),
  -std::numeric_limits< FP >::denorm_min(),
  std::numeric_limits< FP >::epsilon(),
  -std::numeric_limits< FP >::epsilon(),
  std::numeric_limits< FP >::lowest(),
  std::numeric_limits< FP >::max(),
};

#define OPER_PAIR( lambda ) \
  { lambda, #lambda }

template< typename FP, typename INT >
void generate_tests() {
  using FUNC1 = std::pair< FP ( * )( FP ), std::string_view >[];
  for( auto oper_pair : FUNC1{
         OPER_PAIR( [] [[gnu::noinline]] ( auto x ) { return -x; } ),
       } ) {
    for( auto x : special_values< FP, INT > ) {
      generate_test( oper_pair, x );
    }
  }

  using FUNC2 = std::pair< FP ( * )( FP, FP ), std::string_view >[];
  for( auto oper_pair : FUNC2{
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x + y; } ),
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x - y; } ),
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x * y; } ),
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x / y; } ),
       } ) {
    for( auto x : special_values< FP, INT > ) {
      for( auto y : special_values< FP, INT > ) {
        generate_test( oper_pair, x, y );
      }
    }
  }

  using FUNC3 = std::pair< FP ( * )( FP, FP, FP ), std::string_view >[];
  for( auto oper_pair : FUNC3{
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y, auto z ) {
           using namespace std;
           if constexpr( is_same_v< decltype( x ), float > ) {
             return fmaf( x, y, z );
           } else {
             return fma( x, y, z );
           }
         } ),
       } ) {
    for( auto x : special_values< FP, INT > ) {
      for( auto y : special_values< FP, INT > ) {
        for( auto z : special_values< FP, INT > ) {
          generate_test( oper_pair, x, y, z );
        }
      }
    }
  }
}

int main( int argc, char** argv ) {
  if( argc != 2 )
    return 1;
  else if( !strcmp( argv[1], "FE_TONEAREST" ) )
    rounding = FE_TONEAREST;
  else if( !strcmp( argv[1], "FE_UPWARD" ) )
    rounding = FE_UPWARD;
  else if( !strcmp( argv[1], "FE_DOWNWARD" ) )
    rounding = FE_DOWNWARD;
  else if( !strcmp( argv[1], "FE_TOWARDZERO" ) )
    rounding = FE_TOWARDZERO;
  else
    return 1;

  std::cout << "#include \"fenv_test.h\"\n"
               "static unsigned failures;\n"
               "//clang-format off\n"
               "void (*fenv_tests[])() = {\n";

  generate_tests< float, int32_t >();
  generate_tests< double, int32_t >();

  std::cout << "};\n";
  std::cout << "//clang-format on\n";

  std::cout << "\nint main( int argc, char** argv ) {\n";
  std::cout << "  for( auto test: fenv_tests ) test();\n";
  std::cout << "  if(failures){\n";
  std::cout << "    std::cerr << failures << \" failures\\n\";\n";
  std::cout << "    return 1;\n";
  std::cout << "  }else{\n";
  std::cout << "    std::cout << \"All tests passed\\n\";\n";
  std::cout << "  }\n";
  std::cout << "}\n";
}
