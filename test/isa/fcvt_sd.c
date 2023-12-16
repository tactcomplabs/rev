/*
 * fcvt_sd.c
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
    asm volatile( #inst " %0, %1" rm : "=f"(res) : "f"(input) );        \
    if (res != (result))                                                \
      asm volatile(" .word 0; .word " #test);                           \
  } while(0)

int main(int argc, char **argv){
#if __riscv_flen >= 64

  asm volatile("slli x0,x0,1"); // Enable tracing

  FCVT_TEST(          1,   float,    double,   fcvt.s.d,                  -1.125f,        -1.125,   ", rne"  );
  FCVT_TEST(          2,   float,    double,   fcvt.s.d,                  -1.f,             -1.0,   ", rne"  );
  FCVT_TEST(          3,   double,    float,   fcvt.d.s,                 -1.125,         -1.125f,   ""       );
  FCVT_TEST(          4,   double,    float,   fcvt.d.s,                 -1.0,          -1.0f,      ""       );
  FCVT_TEST(          5,   double,    float,   fcvt.d.s,        (double) INFINITY,       INFINITY,  ""       );
  FCVT_TEST(          6,   double,    float,   fcvt.d.s,        (double) -INFINITY,     -INFINITY,  ""       );

#endif // __riscv_flen >= 64

 return 0;
}
