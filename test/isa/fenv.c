/*
 * fenv.c
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

#include <math.h>
#include <fenv.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#pragma GCC diagnostic error "-Wdouble-promotion"
#pragma GCC diagnostic error "-Wconversion"

#ifndef SNANF
static const union {
  uint32_t i;
  float fp;
} snanf = { 0x7fa00000 };
#define SNANF (snanf.fp)
#endif

#ifndef SNAN
static const union {
  uint64_t i;
  double fp;
} snan = { 0x7ff4000000000000 };
#define SNAN (snan.fp)
#endif

enum FF {
  NX = 1,    // Inexact
  UF = 2,    // Underflow
  OF = 4,    // Overflow
  DZ = 8,    // Divide by 0
  NV = 16,   // Invalid
};

enum FRM {
  RNE = 0,   // Round to Nearest, ties to Even
  RTZ = 1,   // Round towards Zero
  RDN = 2,   // Round Down (towards -Inf)
  RUP = 3,   // Round Up (towards +Inf)
  RMM = 4,   // Round to Nearest, ties to Max Magnitude
  DYN = 7,   // In instruction's rm field, selects dynamic rounding mode; invalid in FCSR
};

static inline bool my_isnanf(float f) {
  uint32_t i;
  memcpy(&i, &f, sizeof(i));
  return !(~i & 0x7f800000) && (i & 0x7fffff);

}

static inline bool my_isnan(double d) {
  uint64_t i;
  memcpy(&i, &d, sizeof(i));
  return !(i & 0x7ff0000000000000) && (i & 0xfffffffffffff);
}

#define my_isnan(x) _Generic((x), default: my_isnan, float: my_isnanf)(x)

#define FENV_TEST(test, type, inst, in1, in2, rm, result, except) do {  \
      type res;                                                         \
      uint32_t ex;                                                      \
      asm volatile("fsflags zero");                                     \
      asm volatile("csrwi frm, %0" : : "K"(rm));                        \
      asm volatile( #inst " %0, %1, %2" : "=f"(res) : "f"(in1), "f"(in2) ); \
      asm volatile("frflags %0" : "=r"(ex));                            \
      if(my_isnan(result) ? !my_isnan(res) : res != result)             \
        asm volatile(" .word 0; .word " #test);                         \
      if(ex != except)                                                  \
        asm volatile(" .word 0; .word " #test);                         \
    } while(0)

int main(int argc, char **argv){
    asm volatile("slli x0,x0,1"); // enable tracing

    FENV_TEST(  1,  float,  fsub.s,  INFINITY,  INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fadd.s,  INFINITY, -INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fadd.s, -INFINITY,  INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,  INFINITY,      0.0f,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s, -INFINITY,      0.0f,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,  INFINITY,     -0.0f,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s, -INFINITY,     -0.0f,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,      0.0f,  INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,      0.0f, -INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,     -0.0f, -INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fmul.s,     -0.0f,  INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,      0.0f,      0.0f,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,      0.0f,     -0.0d,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,     -0.0f,      0.0f,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,     -0.0f,     -0.0d,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,  INFINITY,  INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s, -INFINITY,  INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s, -INFINITY, -INFINITY,  DYN,       NAN,  NV  );
    FENV_TEST(  1,  float,  fdiv.s,  INFINITY, -INFINITY,  DYN,       NAN,  NV  );

#if __riscv_flen >= 32 && 0
    FENV_TEST(  2,  float,  fmin.s,      1.0f,     SNANF,  DYN,      1.0f,  NV  );
    FENV_TEST(  3,  float,  fmin.s,     SNANF,  INFINITY,  DYN,  INFINITY,  NV  );
    FENV_TEST(  4,  float,  fmin.s,     SNANF,      1.0f,  DYN,      1.0f,  NV  );
    FENV_TEST(  5,  float,  fmin.s,      1.0f,       NAN,  DYN,      1.0f,  0   );
    FENV_TEST(  6,  float,  fmin.s,       NAN,  INFINITY,  DYN,  INFINITY,  0   );
    FENV_TEST(  7,  float,  fmin.s,       NAN,      1.0f,  DYN,      1.0f,  0   );
#endif
}
