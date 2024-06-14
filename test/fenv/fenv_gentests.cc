/*
 * fenv_gentests.cc
 *
 * RISC-V ISA: RV32F, RV32D, RV64F, RV64D
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include "fenv_test.h"

constexpr unsigned maxtests_per_file = 1000;

static std::string   file_prefix;
static unsigned      testnum;
static unsigned      testcount;
static unsigned      filenum;
static int           rounding;
static std::ofstream out;

static void openfile() {
  ++filenum;
  std::string filename = file_prefix + "_" + std::to_string( filenum );
  out.open( filename + ".cc", std::ios::out | std::ios::trunc );
  if( !out.is_open() ) {
    std::cerr << "Error: Could not open " << filename << std::endl;
    exit( 1 );
  }
  std::cout << " " << filename + ".exe";

  out << R"(
#include "fenv_test.h"

unsigned failures;

void (*fenv_tests[])() = {
//clang-format off
)";
}

static void closefile() {
  if( out.is_open() ) {
    out << R"(
};
//clang-format on
size_t num_fenv_tests = sizeof( fenv_tests ) / sizeof( *fenv_tests );

int main() {
  for( size_t i = 0; i < num_fenv_tests; ++i ) {
    fenv_tests[i]();

    // Make a useless syscall to have fencing effects
#ifdef __riscv
    rev_getuid();
#else
    syscall( SYS_getuid );
#endif
  }

#ifdef __riscv
  if(failures)
    asm(" .word 0");
#endif

  return !!failures;
}
)";
    out.close();
  }
}

template<typename T>
constexpr const char* type = nullptr;
template<>
constexpr const char* type<float> = "float";
template<>
constexpr const char* type<double> = "double";

template<typename T, typename... Ts>
static void generate_test( const std::pair<T ( * )( Ts... ), std::string_view>& oper_pair, Ts... ops ) {
  if( !out.is_open() ) {
    openfile();
  }

  ++testnum;
  auto& [func, func_src] = oper_pair;
  fenv_t fenv;
  std::fesetround( rounding );
  std::feholdexcept( &fenv );

  volatile T result  = func( ops... );
  int        excepts = std::fetestexcept( FE_ALL_EXCEPT );

  out << "  []{\n";
  out << "    // Test " << testnum << "\n";
  out << "    using namespace std;\n";
  out << "    auto func = " << func_src << ";\n";
  out << "    auto func_src = \"" << func_src << "\";\n";
  out << "    fenv_t fenv;\n";

  switch( rounding ) {
  case FE_TONEAREST: out << "    fesetround( FE_TONEAREST );\n"; break;
  case FE_TOWARDZERO: out << "    fesetround( FE_TOWARDZERO );\n"; break;
  case FE_UPWARD: out << "    fesetround( FE_UPWARD );\n"; break;
  case FE_DOWNWARD: out << "    fesetround( FE_DOWNWARD );\n"; break;
  }

  out << "    feholdexcept( &fenv );\n";
  out << "    volatile " << type<T> << " result = func( " << args_string( ops... ) << " );\n";
  out << "    volatile " << type<T> << " dummy = " << type<T> << "(0) + " << type<T> << "(0);\n";
  out << "    int exceptions = fetestexcept( FE_ALL_EXCEPT );\n";
  out << "    auto result_expected = " << float_string( result ) << ";\n";
  out << "    int exceptions_expected = " << exception_string( excepts ) << ";\n";
  out << "    bool result_passed = test_result( \"" << testnum << "\", func_src, result, result_expected, " << args_string( ops... )
      << " );\n";
  out << "    bool exceptions_passed = test_exceptions( \"" << testnum
      << "\", func_src, exceptions, exceptions_expected, result_passed, " << args_string( ops... ) << ");\n";
  out << "    failures += !(result_passed && exceptions_passed);\n";
  out << "    fesetenv( &fenv );\n";
  out << "  },\n";
  std::fesetenv( &fenv );

  if( ++testcount >= maxtests_per_file ) {
    testcount = 0;
    closefile();
  }
}

template<typename FP, typename INT>
constexpr FP special_values[] = {
  0.0f,
  -0.0f,
  1.0f,
  -1.0f,
  1.5f,
  -1.5f,
  (FP) 3.1,
  (FP) -3.1,
  std::numeric_limits<FP>::quiet_NaN(),
  std::numeric_limits<FP>::signaling_NaN(),
  std::numeric_limits<FP>::infinity(),
  -std::numeric_limits<FP>::infinity(),
  std::numeric_limits<FP>::epsilon(),
  -std::numeric_limits<FP>::epsilon(),
  std::numeric_limits<FP>::lowest(),
  std::numeric_limits<FP>::max(),
};

#define OPER_PAIR( lambda ) \
  { lambda, #lambda }

template<typename FP, typename INT>
static void generate_tests() {
  using FUNC1 = std::pair<FP ( * )( FP ), std::string_view>[];
  for( auto oper_pair : FUNC1{
         OPER_PAIR( []( volatile auto x ) { return -x; } ),
       } ) {
    for( auto x : special_values<FP, INT> ) {
      generate_test( oper_pair, x );
    }
  }

  using FUNC2 = std::pair<FP ( * )( FP, FP ), std::string_view>[];
  for( auto oper_pair : FUNC2{
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x + y; } ),
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x - y; } ),
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x * y; } ),
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x / y; } ),
       } ) {
    for( auto x : special_values<FP, INT> ) {
      for( auto y : special_values<FP, INT> ) {
        generate_test( oper_pair, x, y );
      }
    }
  }

  using FUNC3 = std::pair<FP ( * )( FP, FP, FP ), std::string_view>[];
  for( auto oper_pair : FUNC3{
         OPER_PAIR( ( []( volatile auto x, volatile auto y, volatile auto z ) {
           using namespace std;
           using T = common_type_t<decltype( x ), decltype( y ), decltype( z )>;
           if constexpr( is_same<T, float>::value ) {
             return fmaf( T( x ), T( y ), T( z ) );
           } else {
             return fma( T( x ), T( y ), T( z ) );
           }
         } ) ),
       } ) {
    for( auto x : special_values<FP, INT> ) {
      for( auto y : special_values<FP, INT> ) {
        for( auto z : special_values<FP, INT> ) {
          generate_test( oper_pair, x, y, z );
        }
      }
    }
  }
}

[[noreturn]] static void usage( const char* prog ) {
  std::cerr << "Usage: " << prog << "{ FE_TONEAREST | FE_UPWARD | FE_DOWNWARD | FE_TOWARDZERO }" << std::endl;
  exit( 1 );
}

int main( int argc, char** argv ) {
  if( argc < 2 )
    usage( *argv );
  else if( !strcmp( argv[1], "FE_TONEAREST" ) )
    rounding = FE_TONEAREST;
  else if( !strcmp( argv[1], "FE_UPWARD" ) )
    rounding = FE_UPWARD;
  else if( !strcmp( argv[1], "FE_DOWNWARD" ) )
    rounding = FE_DOWNWARD;
  else if( !strcmp( argv[1], "FE_TOWARDZERO" ) )
    rounding = FE_TOWARDZERO;
  else
    usage( *argv );

  file_prefix = argv[1];

  generate_tests<float, int32_t>();
  generate_tests<double, int32_t>();

  closefile();

  return 0;
}
