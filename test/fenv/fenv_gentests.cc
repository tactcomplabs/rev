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

#if 0

#include <array>
#include <cfenv>
#include <cinttypes>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

enum FF {
  NX = 1,   // Inexact
  UF = 2,   // Underflow
  OF = 4,   // Overflow
  DZ = 8,   // Divide by 0
  NV = 16,  // Invalid
};

enum FRM {
  RNE = 0,  // Round to Nearest, ties to Even
  RTZ = 1,  // Round towards Zero
  RDN = 2,  // Round Down (towards -Inf)
  RUP = 3,  // Round Up (towards +Inf)
  RMM = 4,  // Round to Nearest, ties to Max Magnitude
  DYN =
    7,  // In instruction's rm field, selects dynamic rounding mode; invalid in FCSR
};

#define my_isnan( x ) \
  _Generic( ( x ), default : my_isnand, float : my_isnanf )( x )

#define FENV_TEST( test, type, inst, in1, in2, rm, result, except ) \
  do {                                                              \
    type     res;                                                   \
    uint32_t ex;                                                    \
    asm volatile( "fsflags zero" );                                 \
    asm volatile( "csrwi frm, %0" : : "K"( rm ) );                  \
    asm volatile( #inst " %0, %1, %2"                               \
                  : "=f"( res )                                     \
                  : "f"( in1 ), "f"( in2 ) );                       \
    asm volatile( "frflags %0" : "=r"( ex ) );                      \
    if( ex != except )                                              \
      asm volatile( " .word 0; .word " #test );                     \
    if( my_isnan( result ) ? !my_isnan( res ) : res != result )     \
      asm volatile( " .word 0; .word " #test );                     \
  } while( 0 )

#if 0
int main(int argc, char **argv){
#if __riscv_flen >= 32

    FENV_TEST(  1,  float,  fsub.s,  INFINITY,  INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fadd.s,  INFINITY, -INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fadd.s, -INFINITY,  INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,  INFINITY,      0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s, -INFINITY,      0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,  INFINITY,     -0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s, -INFINITY,     -0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,      0.0f,  INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,      0.0f, -INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,     -0.0f, -INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,     -0.0f,  INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,      0.0f,      0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,      0.0f,     -0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,     -0.0f,      0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,     -0.0f,     -0.0f,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,  INFINITY,  INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s, -INFINITY,  INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s, -INFINITY, -INFINITY,  RNE,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,  INFINITY, -INFINITY,  RNE,       NAN,  NV  );

    FENV_TEST(  2,  float,  fmin.s,      1.0f,     SNANF,  RNE,      1.0f,  NV  );
    FENV_TEST(  3,  float,  fmin.s,     SNANF,  INFINITY,  RNE,  INFINITY,  NV  );
    FENV_TEST(  4,  float,  fmin.s,     SNANF,      1.0f,  RNE,      1.0f,  NV  );
    FENV_TEST(  5,  float,  fmin.s,      1.0f,       NAN,  RNE,      1.0f,  0   );
    FENV_TEST(  6,  float,  fmin.s,       NAN,  INFINITY,  RNE,  INFINITY,  0   );
    FENV_TEST(  7,  float,  fmin.s,       NAN,      1.0f,  RNE,      1.0f,  0   );


#endif

}

#endif

template<typename T>
auto& print_float(T x){
  const char* type = std::is_same_v<T, float> ? "float" : "double";
  if(std::isnan(x)){
    std::cout << "std::numeric_limits<" << type << ">::";
    std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t> ix;
    std::memcpy(&ix, &x, sizeof(ix));
    if(ix & decltype(ix){1}<<(std::is_same_v<T, float> ? 22 : 51)){
      return std::cout << "quiet_NaN()";
    }else{
      return std::cout << "signaling_NaN()";
    }
  }else if(std::isinf(x)){
    return std::cout << (x < 0 ? "-" : "") << "std::numeric_limits<" << type << ">::infinity()";
  }else{
    return std::cout << x << (std::is_same_v<T,float> ? "f" : "");
  }
}

#endif

static unsigned testno, failures;

template< typename T, typename... Ts >
void generate_test(
  const std::pair< std::function< T( Ts... ) >, std::string_view >& oper_pair,
  Ts... ops ) {
  auto& [func, func_src] = oper_pair;
  fenv_t fenv;
  std::feholdexcept( &fenv );
  T   result  = func( ops... );
  int excepts = std::fetestexcept( FE_ALL_EXCEPT );

  std::cout << "  {\n";
  std::cout << "    // Test " << ++testno << "\n";
  std::cout << "    auto func = " << func_src << ";\n";
  std::cout << "    auto func_src = \"" << func_src << "\";\n";
  std::cout << "    std::fenv_t env;\n";
  std::cout << "    std::feholdexcept( &env );\n";
  std::cout << "    auto result = func( " << args_string( ops... ) << " );\n";
  std::cout << "    int exceptions = std::fetestexcept( FE_ALL_EXCEPT );\n";
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
  std::cout << "  }\n";
  std::fesetenv( &fenv );
}

template< typename FP >
constexpr FP special_values[] = {
  0.0f,
  -0.0f,
  1.0f,
  -1.0f,
  1.5f,
  -1.5f,
  3.1,
  -3.1,
  std::numeric_limits< FP >::quiet_NaN(),
  std::numeric_limits< FP >::signaling_NaN(),
  std::numeric_limits< FP >::infinity(),
  -std::numeric_limits< FP >::infinity(),
  std::numeric_limits< FP >::denorm_min(),
  -std::numeric_limits< FP >::denorm_min(),
  std::numeric_limits< FP >::epsilon(),
  -std::numeric_limits< FP >::epsilon(),
  std::numeric_limits< FP >::denorm_min(),
  -std::numeric_limits< FP >::denorm_min(),
  std::numeric_limits< FP >::lowest(),
  std::numeric_limits< FP >::max(),
};

#define OPER_PAIR( lambda ) \
  { lambda, #lambda }

template< typename FP, typename INT >
void generate_tests() {
  std::cout << "#include \"fenv_test.h\"\n"
               "static unsigned failures;\n"
               "//clang-format off\n"
               "void fenv_tests(){\n";

  using FUNC1 = std::pair< std::function< FP( FP ) >, std::string_view >[];
  for( auto oper_pair : FUNC1{
         OPER_PAIR( [] [[gnu::noinline]] ( auto x ) { return -x; } ),
       } ) {
    for( auto x : special_values< FP > ) {
      generate_test( oper_pair, x );
    }
  }

  using FUNC2 = std::pair< std::function< FP( FP, FP ) >, std::string_view >[];
  for( auto oper_pair : FUNC2{
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x + y; } ),
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x - y; } ),
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x * y; } ),
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y ) { return x / y; } ),
       } ) {
    for( auto x : special_values< FP > ) {
      for( auto y : special_values< FP > ) {
        generate_test( oper_pair, x, y );
      }
    }
  }

  using FUNC3 =
    std::pair< std::function< FP( FP, FP, FP ) >, std::string_view >[];
  for( auto oper_pair : FUNC3{
         OPER_PAIR( [] [[gnu::noinline]] ( auto x, auto y, auto z ) {
           return std::fma( x, y, z );
         } ),
       } ) {
    for( auto x : special_values< FP > ) {
      for( auto y : special_values< FP > ) {
        for( auto z : special_values< FP > ) {
          generate_test( oper_pair, x, y, z );
        }
      }
    }
  }

  std::cout << "}\n";
  std::cout << "//clang-format on\n";

  std::cout << "\nint main() {\n";
  std::cout << "  fenv_tests();\n";
  std::cout << "  if(failures){\n";
  std::cout << "    std::cerr << failures << \" failures\\n\";\n";
  std::cout << "    return 1;\n";
  std::cout << "  }else{\n";
  std::cout << "    std::cout << \"All tests passed\\n\";\n";
  std::cout << "  }\n";
  std::cout << "}\n";
}

int main() {
  generate_tests< double, int32_t >();
}
