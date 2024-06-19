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

const char*   file_prefix;
size_t        testnum;
size_t        testcount;
unsigned      filenum;
int           rounding;
std::ofstream out;
size_t        failures = 0;

void openfile() {
  ++filenum;
  char filename[256];
  char filenum_s[32];
  snprintf( filenum_s, sizeof( filenum_s ), "%u", filenum );
  strcat( strcat( strcpy( filename, file_prefix ), "_" ), filenum_s );
  std::cout << " " << filename << ".exe";
  strcat( filename, ".cc" );
  out.open( filename, std::ios::out | std::ios::trunc );
  if( !out.is_open() ) {
    std::cerr << "Error: Could not open " << filename << std::endl;
    exit( 1 );
  }

  out << R"(
#include "fenv_test.h"

size_t failures = 0;

constexpr char file_prefix[] = ")"
      << file_prefix << R"(";

void (*fenv_tests[])() = {
)";
}

void closefile() {
  if( out.is_open() ) {
    out << R"(
};

size_t num_fenv_tests = sizeof( fenv_tests ) / sizeof( *fenv_tests );

int main() {
  for( size_t i = 0; i < num_fenv_tests; ++i )
    fenv_tests[i]();

  char fail[64];
  snprintf( fail, sizeof(fail), "\nfenv %s: %zu / )"
        << testcount << R"( test failures\n", file_prefix, failures );
  fenv_write( 2, fail );

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

template<typename T, typename... Ts>
void generate_test( const std::pair<T ( * )( Ts... ), const char*>& oper_pair, Ts... ops ) {
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
  out << "    auto func_src = R\"(" << func_src << "\n)\";\n";
  out << "    fenv_t fenv;\n";

  switch( rounding ) {
  case FE_TONEAREST: out << "    fesetround( FE_TONEAREST );\n"; break;
  case FE_TOWARDZERO: out << "    fesetround( FE_TOWARDZERO );\n"; break;
  case FE_UPWARD: out << "    fesetround( FE_UPWARD );\n"; break;
  case FE_DOWNWARD: out << "    fesetround( FE_DOWNWARD );\n"; break;
  }

  out << "    feholdexcept( &fenv );\n";
  out << "    " << type<T> << " result{ func( " << args_string( ops... ) << " ) };\n";
  out << "    int exceptions = fetestexcept( FE_ALL_EXCEPT );\n";
  out << "    " << type<T> << " result_expected{ " << repr( result ) << " };\n";
  out << "    int exceptions_expected = " << exception_string( excepts ) << ";\n";
  out << "    bool result_passed = test_result( \"" << testnum << "\", file_prefix, func_src, result, result_expected, "
      << args_string( ops... ) << " );\n";
  out << "    bool exceptions_passed = test_exceptions( \"" << testnum
      << "\", file_prefix, func_src, exceptions, exceptions_expected, result_passed, " << args_string( ops... ) << ");\n";
  out << "    failures += !(result_passed && exceptions_passed);\n";
  out << "    fesetenv( &fenv );\n";
  out << "  },\n";
  std::fesetenv( &fenv );

  if( ++testcount >= maxtests_per_file ) {
    closefile();
    testcount = 0;
  }
}

template<typename FP>
constexpr FP special_fp_values[] = {
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

template<typename FP, typename INT>
constexpr FP special_fcvt_values[] = {
  FP( std::numeric_limits<INT>::max() ),
  FP( std::numeric_limits<INT>::max() ) + FP( 0.5 ),
  FP( std::numeric_limits<INT>::max() ) + FP( 1.0 ),
  FP( std::numeric_limits<INT>::max() ) - FP( 0.5 ),
  FP( std::numeric_limits<INT>::max() ) - FP( 1.0 ),
  FP( std::numeric_limits<INT>::min() ),
  FP( std::numeric_limits<INT>::min() ) + FP( 0.5 ),
  FP( std::numeric_limits<INT>::min() ) + FP( 1.0 ),
  FP( std::numeric_limits<INT>::min() ) - FP( 0.5 ),
  FP( std::numeric_limits<INT>::min() ) - FP( 1.0 ),
  FP( 0 ),
  -FP( 0 ),
  FP( 0.1 ),
  FP( -0.1 ),
  FP( 0.5 ),
  FP( -0.5 ),
  FP( 0.9 ),
  FP( -0.9 ),
  std::numeric_limits<FP>::quiet_NaN(),
  std::numeric_limits<FP>::signaling_NaN(),
  std::numeric_limits<FP>::infinity(),
  -std::numeric_limits<FP>::infinity(),
};

#define OPER_PAIR( lambda ) \
  { lambda, #lambda }

template<typename FP, typename INT>
void generate_tests() {
  using FUNC1 = std::pair<FP ( * )( FP ), const char*>[];
  for( auto oper_pair : FUNC1{
         OPER_PAIR( []( volatile auto x ) { return -x; } ),
         OPER_PAIR( []( volatile auto x ) { return std::fabs( x ); } ),
       } ) {
    for( auto x : special_fp_values<FP> ) {
      generate_test( oper_pair, x );
    }
  }

  char test_src[256];
  test_src[0] = 0;
  strcat( test_src, "[]( volatile auto x ) { return to_int<" );
  strcat( test_src, type<INT> );
  strcat( test_src, ">(x); }" );

  using INT_FUNC1 = std::pair<INT ( * )( FP ), const char*>[];
  for( auto oper_pair : INT_FUNC1{
         {[]( volatile auto x ) { return to_int<INT>( x ); }, test_src},
  } ) {
    for( auto x : special_fcvt_values<FP, INT> ) {
      generate_test( oper_pair, x );
    }
  }

  using FUNC2 = std::pair<FP ( * )( FP, FP ), const char*>[];
  for( auto oper_pair : FUNC2{
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x + y; } ),
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x - y; } ),
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x * y; } ),
         OPER_PAIR( []( volatile auto x, volatile auto y ) { return x / y; } ),
       } ) {
    for( auto x : special_fp_values<FP> ) {
      for( auto y : special_fp_values<FP> ) {
        generate_test( oper_pair, x, y );
      }
    }
  }

  using FUNC3 = std::pair<FP ( * )( FP, FP, FP ), const char*>[];
  for( auto oper_pair : FUNC3{
      {
#if 0
                      []( volatile auto x, volatile auto y, volatile auto z ) {
                        using namespace std;
                        using T = common_type_t<decltype( x ), decltype( y ), decltype( z )>;
                        if constexpr( is_same_v<T, float> ) {
                          return fmaf( T( x ), T( y ), T( z ) );
                        } else {
                          return fma( T( x ), T( y ), T( z ) );
                        }
                      }
#else
           []( volatile auto x, volatile auto y, volatile auto z ) {
             using namespace std;
             using T = common_type_t<decltype( x ), decltype( y ), decltype( z )>;
             return revFMA( T( x ), T( y ), T( z ) );
#endif
                      }
                        ,
R"(
                      []( volatile auto x, volatile auto y, volatile auto z ) {
                        using namespace std;
                        using T = common_type_t<decltype( x ), decltype( y ), decltype( z )>;
                        if constexpr( is_same_v<T, float> ) {
                          return fmaf( T( x ), T( y ), T( z ) );
                        } else {
                          return fma( T( x ), T( y ), T( z ) );
                        }
                      }
)"
                        },
} ) {
  for( auto x : special_fp_values<FP> ) {
    for( auto y : special_fp_values<FP> ) {
      for( auto z : special_fp_values<FP> ) {
        generate_test( oper_pair, x, y, z );
      }
    }
  }
}
}

[[noreturn]] void usage( const char* prog ) {
  std::cerr << "Usage: " << prog << "{ TONEAREST | UPWARD | DOWNWARD | TOWARDZERO }" << std::endl;
  exit( 1 );
}

int main( int argc, char** argv ) {
  if( argc < 2 )
    usage( *argv );
  else if( !strcmp( argv[1], "TONEAREST" ) )
    rounding = FE_TONEAREST;
  else if( !strcmp( argv[1], "UPWARD" ) )
    rounding = FE_UPWARD;
  else if( !strcmp( argv[1], "DOWNWARD" ) )
    rounding = FE_DOWNWARD;
  else if( !strcmp( argv[1], "TOWARDZERO" ) )
    rounding = FE_TOWARDZERO;
  else
    usage( *argv );

  file_prefix = argv[1];

  std::cout << file_prefix << ":";
  generate_tests<float, int32_t>();
  generate_tests<double, int32_t>();
  std::cout << "\n\t$(foreach exe,$^,./run_fenv_test.sh $(exe) &&) true\n.PHONY: " << file_prefix << std::endl;
  closefile();

  return 0;
}
