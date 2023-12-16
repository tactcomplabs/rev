/*
 * fcvt_sw.c
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
#include <stdint.h>
#include <math.h>

#pragma GCC diagnostic error "-Wdouble-promotion"
#pragma GCC diagnostic error "-Wconversion"

#define FCVT_TEST(test, to_t, from_t, inst, result, input, rm)  do {    \
    to_t res = (to_t) 0xE0E0E0E0;                                       \
    asm volatile( #inst " %0, %1, " #rm : "=f"(res) : "r"(input) );     \
    if (res != (result))                                                \
      asm(" .word 0; .word " #test);                                    \
  } while(0)

int main(int argc, char **argv){
  asm volatile("slli x0,x0,1"); // Enable tracing

#if __riscv_flen >= 32

  FCVT_TEST(   1,   float,   int32_t,   fcvt.s.w,       1.f,    (int32_t)1,  rne  );
  FCVT_TEST(   2,   float,   int32_t,   fcvt.s.w,      -1.f,   -(int32_t)1,  rne  );
  FCVT_TEST(   3,   float,   int32_t,   fcvt.s.w,     -0.0f,    (int32_t)0,  rne  );
  FCVT_TEST(   4,   float,  uint32_t,  fcvt.s.wu,      1.0f,   (uint32_t)1,  rne  );
  FCVT_TEST(   5,   float,  uint32_t,  fcvt.s.wu,  0x1p+32f,  -(uint32_t)1,  rne  );
  FCVT_TEST(   6,   float,  uint32_t,  fcvt.s.wu,      0.0f,   (uint32_t)0,  rne  );

#if __riscv_xlen >= 64

  FCVT_TEST(   7,   float,   int64_t,   fcvt.s.l,      1.0f,    (int64_t)1,  rne  );
  FCVT_TEST(   8,   float,   int64_t,   fcvt.s.l,     -1.0f,   -(int64_t)1,  rne  );
  FCVT_TEST(   9,   float,   int64_t,   fcvt.s.l,      0.0f,    (int64_t)0,  rne  );
  FCVT_TEST(  10,   float,  uint64_t,  fcvt.s.lu,      1.0f,   (uint64_t)1,  rne  );
  FCVT_TEST(  11,   float,  uint64_t,  fcvt.s.lu,  0x1p+64f,  -(uint64_t)1,  rne  );
  FCVT_TEST(  12,   float,  uint64_t,  fcvt.s.lu,      0.0f,   (uint64_t)0,  rne  );

#if __riscv_flen >= 64

  FCVT_TEST(  13,  double,   int64_t,   fcvt.d.l,       1.0,    (int64_t)1,  rne  );
  FCVT_TEST(  14,  double,   int64_t,   fcvt.d.l,      -1.0,   -(int64_t)1,  rne  );
  FCVT_TEST(  15,  double,   int64_t,   fcvt.d.l,       0.0,    (int64_t)0,  rne  );
  FCVT_TEST(  16,  double,  uint64_t,  fcvt.d.lu,       1.0,   (uint64_t)1,  rne  );
  FCVT_TEST(  17,  double,  uint64_t,  fcvt.d.lu,   0x1p+64,  -(uint64_t)1,  rne  );
  FCVT_TEST(  18,  double,  uint64_t,  fcvt.d.lu,       0.0,   (uint64_t)0,  rne  );

#endif // __riscv_flen >= 64
#endif // __riscv_xlen >= 64
#endif // __riscv_flen >= 32

 return 0;
}
