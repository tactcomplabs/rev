/*
 * fcvt.c
 *
 * RISC-V ISA: RV32I
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "isa_test_macros.h"

int main(int argc, char **argv){

  // #-------------------------------------------------------------
  // # Arithmetic tests
  // #-------------------------------------------------------------

  TEST_INT_FP_OP_S( 2,  fcvt.s.w,                   2.0,  2);
  TEST_INT_FP_OP_S( 3,  fcvt.s.w,                  -2.0, -2);

  TEST_INT_FP_OP_S( 4, fcvt.s.wu,                   2.0,  2);
  TEST_INT_FP_OP_S( 5, fcvt.s.wu,           4.2949673e9, -2);

#if __riscv_xlen >= 64
  TEST_INT_FP_OP_S( 6,  fcvt.s.l,                   2.0,  2);
  TEST_INT_FP_OP_S( 7,  fcvt.s.l,                  -2.0, -2);

  TEST_INT_FP_OP_S( 8, fcvt.s.lu,                   2.0,  2);
  TEST_INT_FP_OP_S( 9, fcvt.s.lu,          1.8446744e19, -2);
#endif

  TEST_INT_FP_OP_D(10,  fcvt.d.w,                   2.0,  2);
  TEST_INT_FP_OP_D(11,  fcvt.d.w,                  -2.0, -2);

  TEST_INT_FP_OP_D(12, fcvt.d.wu,                   2.0,  2);
  TEST_INT_FP_OP_D(13, fcvt.d.wu,            4294967294, -2);

#if __riscv_xlen >= 64
  TEST_INT_FP_OP_D(14,  fcvt.d.l,                   2.0,  2);
  TEST_INT_FP_OP_D(15,  fcvt.d.l,                  -2.0, -2);

  TEST_INT_FP_OP_D(16, fcvt.d.lu,                   2.0,  2);
  TEST_INT_FP_OP_D(17, fcvt.d.lu, 1.8446744073709552e19, -2);
#endif


  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(0);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}

asm(".data");
RVTEST_DATA_BEGIN
  TEST_FP_INT_OP_DATA1( 2,                  2.0,  2);
  TEST_FP_INT_OP_DATA1( 3,                 -2.0, -2);

  TEST_FP_INT_OP_DATA1( 4,                 2.0,  2);
  TEST_FP_INT_OP_DATA1( 5,         4.2949673e9, -2);

  TEST_FP_INT_OP_DATA1( 6,                  2.0,  2);
  TEST_FP_INT_OP_DATA1( 7,                 -2.0, -2);

  TEST_FP_INT_OP_DATA1( 8,                 2.0,  2);
  TEST_FP_INT_OP_DATA1( 9,        1.8446744e19, -2);

  TEST_FP_D_INT_OP_DATA1( 10,                 2.0,  2);
  TEST_FP_D_INT_OP_DATA1( 11,                 -2.0, -2);

  TEST_FP_D_INT_OP_DATA1( 12,                 2.0,  2);
  TEST_FP_D_INT_OP_DATA1( 13,                 4294967294.0,  -2);

  TEST_FP_D_INT_OP_DATA1( 14,                  2.0,  2);
  TEST_FP_D_INT_OP_DATA1( 15,                 -2.0, -2);

  TEST_FP_D_INT_OP_DATA1( 16,                 2.0,  2);
  TEST_FP_D_INT_OP_DATA1( 17,        1.8446744073709552e19, -2);
RVTEST_DATA_END
