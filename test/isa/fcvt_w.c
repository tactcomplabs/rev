/*
77;30700;0c * fcvt_w.c
 *
 * RISC-V ISA: RV32I
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */
#include <math.h>
#include <stdint.h>

#pragma GCC diagnostic error "-Wdouble-promotion"
#pragma GCC diagnostic error "-Wconversion"

#define FCVT_TEST( test, to_t, from_t, inst, result, input, rm )        \
  do {                                                                  \
    to_t res = (to_t) 0xE0E0E0E0;                                       \
    asm volatile( #inst " %0, %1, " #rm : "=r"( res ) : "f"( input ) ); \
    if( res != ( result ) )                                             \
      asm volatile( " .word 0; .word " #test );                         \
  } while( 0 )

int main( int argc, char** argv ) {
  asm volatile( "slli x0,x0,1" );  // Enable tracing

#if __riscv_flen >= 32

  FCVT_TEST( 2, int32_t, float, fcvt.w.s, -1, -1.1f, rtz );
  FCVT_TEST( 3, int32_t, float, fcvt.w.s, -1, -1.0f, rtz );
  FCVT_TEST( 4, int32_t, float, fcvt.w.s, 0, -0.9f, rtz );
  FCVT_TEST( 5, int32_t, float, fcvt.w.s, 0, 0.9f, rtz );
  FCVT_TEST( 6, int32_t, float, fcvt.w.s, 1, 1.0f, rtz );
  FCVT_TEST( 7, int32_t, float, fcvt.w.s, 1, 1.1f, rtz );
  // FCVT_TEST(         8,   int32_t,     float,   fcvt.w.s,          0x7fffff80,   0x1.fffffep+30,  rtz  );
  // FCVT_TEST(         9,   int32_t,     float,   fcvt.w.s,           INT32_MAX,  0x1.ffffffp+30f,  rtz  );
  FCVT_TEST( 10, int32_t, float, fcvt.w.s, 0x80000080, -0x1.fffffep+30f, rtz );
  FCVT_TEST( 11, int32_t, float, fcvt.w.s, INT32_MIN, -0x1.ffffffp+30f, rtz );
  FCVT_TEST( 12, uint32_t, float, fcvt.wu.s, 0, -1.1f, rtz );
  FCVT_TEST( 13, uint32_t, float, fcvt.wu.s, 0, -1.0f, rtz );
  FCVT_TEST( 14, uint32_t, float, fcvt.wu.s, 0, -3.0f, rtz );
  FCVT_TEST( 15, uint32_t, float, fcvt.wu.s, 0, -1.0f, rtz );
  FCVT_TEST( 16, uint32_t, float, fcvt.wu.s, 0, -0.9f, rtz );
  FCVT_TEST( 17, uint32_t, float, fcvt.wu.s, 0, 0.9f, rtz );
  FCVT_TEST( 18, uint32_t, float, fcvt.wu.s, 1, 1.0f, rtz );
  FCVT_TEST( 19, uint32_t, float, fcvt.wu.s, 1, 1.1f, rtz );
  FCVT_TEST( 20, uint32_t, float, fcvt.wu.s, 0, -3e9f, rtz );
  FCVT_TEST( 21, uint32_t, float, fcvt.wu.s, 0xffffff00, 0x1.fffffep+31f, rtz );
  // FCVT_TEST(        22,  uint32_t,     float,  fcvt.wu.s,          UINT32_MAX,  0x1.ffffffp+31f,  rtz  );

#if __riscv_xlen >= 64
  FCVT_TEST( 23, int64_t, float, fcvt.l.s, -1, -1.1f, rtz );
  FCVT_TEST( 24, int64_t, float, fcvt.l.s, -1, -1.0f, rtz );
  FCVT_TEST( 25, int64_t, float, fcvt.l.s, 0, -0.9f, rtz );
  FCVT_TEST( 26, int64_t, float, fcvt.l.s, 0, 0.9f, rtz );
  FCVT_TEST( 27, int64_t, float, fcvt.l.s, 1, 1.0f, rtz );
  FCVT_TEST( 28, int64_t, float, fcvt.l.s, 1, 1.1f, rtz );
  FCVT_TEST( 29, int64_t, float, fcvt.l.s, 0x7fffff8000000000, 0x1.fffffep+62f, rtz );
  // FCVT_TEST(        30,   int64_t,     float,   fcvt.l.s,           INT64_MAX,  0x1.ffffffp+62f,  rtz  );
  FCVT_TEST( 31, int64_t, float, fcvt.l.s, 0x8000010000000000, -0x1.fffffdp+62f, rtz );
  FCVT_TEST( 32, int64_t, float, fcvt.l.s, 0x8000008000000000, -0x1.fffffep+62f, rtz );
  FCVT_TEST( 33, int64_t, float, fcvt.l.s, INT64_MIN, -0x1.ffffffp+62f, rtz );
  FCVT_TEST( 34, uint64_t, float, fcvt.lu.s, 0, -3.0f, rtz );
  FCVT_TEST( 35, uint64_t, float, fcvt.lu.s, 0, -1.0f, rtz );
  FCVT_TEST( 36, uint64_t, float, fcvt.lu.s, 0, -0.9f, rtz );
  FCVT_TEST( 37, uint64_t, float, fcvt.lu.s, 0, 0.9f, rtz );
  FCVT_TEST( 38, uint64_t, float, fcvt.lu.s, 1, 1.0f, rtz );
  FCVT_TEST( 39, uint64_t, float, fcvt.lu.s, 1, 1.1f, rtz );
  FCVT_TEST( 40, uint64_t, float, fcvt.lu.s, 0, -3e9f, rtz );
  FCVT_TEST( 41, uint64_t, float, fcvt.lu.s, 0xffffff0000000000, 0x1.fffffep+63f, rtz );
  // FCVT_TEST(        42,  uint64_t,     float,  fcvt.lu.s,          UINT64_MAX,  0x1.ffffffp+63f,  rtz  );
  FCVT_TEST( 43, uint64_t, float, fcvt.lu.s, 0, -0x1.fffffdp+63f, rtz );
  FCVT_TEST( 44, uint64_t, float, fcvt.lu.s, 0, -0x1.fffffep+63f, rtz );
  FCVT_TEST( 45, uint64_t, float, fcvt.lu.s, 0, -0x1.ffffffp+63f, rtz );

#endif  // __riscv_xlen >= 64

#endif  // __riscv_flen >= 32

#if __riscv_flen >= 64

  FCVT_TEST( 46, int32_t, double, fcvt.w.d, -1, -1.1, rtz );
  FCVT_TEST( 47, int32_t, double, fcvt.w.d, -1, -1.0, rtz );
  FCVT_TEST( 48, int32_t, double, fcvt.w.d, 0, -0.9, rtz );
  FCVT_TEST( 49, int32_t, double, fcvt.w.d, 0, 0.9, rtz );
  FCVT_TEST( 50, int32_t, double, fcvt.w.d, 1, 1.0, rtz );
  FCVT_TEST( 51, int32_t, double, fcvt.w.d, 1, 1.1, rtz );
  FCVT_TEST( 52, int32_t, double, fcvt.w.d, 2147483647, 0x1.fffffffffffffp+30, rtz );
  FCVT_TEST( 53, int32_t, double, fcvt.w.d, INT32_MAX, 0x1.0p+31, rtz );
  FCVT_TEST( 54, int32_t, double, fcvt.w.d, -2147483647, -0x1.fffffffffffffp+30, rtz );
  FCVT_TEST( 55, int32_t, double, fcvt.w.d, INT32_MIN, -0x1.0p+31, rtz );
  FCVT_TEST( 56, uint32_t, double, fcvt.wu.d, 0, -1.1, rtz );
  FCVT_TEST( 57, uint32_t, double, fcvt.wu.d, 0, -1.0, rtz );
  FCVT_TEST( 58, uint32_t, double, fcvt.wu.d, 0, -3.0, rtz );
  FCVT_TEST( 59, uint32_t, double, fcvt.wu.d, 0, -1.0, rtz );
  FCVT_TEST( 60, uint32_t, double, fcvt.wu.d, 0, -0.9, rtz );
  FCVT_TEST( 61, uint32_t, double, fcvt.wu.d, 0, 0.9, rtz );
  FCVT_TEST( 62, uint32_t, double, fcvt.wu.d, 1, 1.0, rtz );
  FCVT_TEST( 63, uint32_t, double, fcvt.wu.d, 1, 1.1, rtz );
  FCVT_TEST( 64, uint32_t, double, fcvt.wu.d, 0, -3e9, rtz );
  FCVT_TEST( 65, uint32_t, double, fcvt.wu.d, UINT32_MAX, 0x1.fffffffffffffp+31, rtz );
  FCVT_TEST( 66, uint32_t, double, fcvt.wu.d, -2, 0x1.fffffffcp+31, rtz );
  FCVT_TEST( 67, uint32_t, double, fcvt.wu.d, UINT32_MAX, 0x1.0p+32, rtz );

#if __riscv_xlen >= 64

  FCVT_TEST( 68, int64_t, double, fcvt.l.d, -1, -1.1, rtz );
  FCVT_TEST( 69, int64_t, double, fcvt.l.d, -1, -1.0, rtz );
  FCVT_TEST( 70, int64_t, double, fcvt.l.d, 0, -0.9, rtz );
  FCVT_TEST( 71, int64_t, double, fcvt.l.d, 0, 0.9, rtz );
  FCVT_TEST( 72, int64_t, double, fcvt.l.d, 1, 1.0, rtz );
  FCVT_TEST( 73, int64_t, double, fcvt.l.d, 1, 1.1, rtz );
  // FCVT_TEST(        74,   int64_t,     double,  fcvt.l.d,  0x7ffffffffffffc00,  0x1.fffffffffffffp+62,  rtz  );
  // FCVT_TEST(        75,   int64_t,     double,  fcvt.l.d,           INT64_MAX,        0x1.0p+63,  rtz  );
  FCVT_TEST( 76, int64_t, double, fcvt.l.d, 0x8000000000000400, -0x1.fffffffffffffp+62, rtz );
  FCVT_TEST( 77, int64_t, double, fcvt.l.d, INT64_MIN, -0x1.0p+63, rtz );
  FCVT_TEST( 78, uint64_t, double, fcvt.lu.d, 0, -3.0, rtz );
  FCVT_TEST( 79, uint64_t, double, fcvt.lu.d, 0, -1.0, rtz );
  FCVT_TEST( 80, uint64_t, double, fcvt.lu.d, 0, -0.9, rtz );
  FCVT_TEST( 81, uint64_t, double, fcvt.lu.d, 0, 0.9, rtz );
  FCVT_TEST( 82, uint64_t, double, fcvt.lu.d, 1, 1.0, rtz );
  FCVT_TEST( 83, uint64_t, double, fcvt.lu.d, 1, 1.1, rtz );
  FCVT_TEST( 84, uint64_t, double, fcvt.lu.d, 0, -3e9, rtz );
  FCVT_TEST( 85, uint64_t, double, fcvt.lu.d, 0xfffffffffffff800, 0x1.fffffffffffffp+63, rtz );
  FCVT_TEST( 86, uint64_t, double, fcvt.lu.d, 0, -0x1.fffffffffffffp+63, rtz );
  FCVT_TEST( 87, uint64_t, double, fcvt.lu.d, 0, -0x1.0p+64, rtz );
  // FCVT_TEST(        88,  uint64_t,    double,  fcvt.lu.d,          UINT64_MAX,        0x1.0p+64,  rtz  );

#endif  // __riscv_xlen >= 64

#endif  // __riscv_flen >= 64
}
