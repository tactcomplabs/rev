/*
 * fadd.c
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

  TEST_FP_INT_OP_S( 2,  fcvt.w.s, 0x01,         -1, -1.1, rtz);
  TEST_FP_INT_OP_S( 3,  fcvt.w.s, 0x00,         -1, -1.0, rtz);
  TEST_FP_INT_OP_S( 4,  fcvt.w.s, 0x01,          0, -0.9, rtz);
  TEST_FP_INT_OP_S( 5,  fcvt.w.s, 0x01,          0,  0.9, rtz);
  TEST_FP_INT_OP_S( 6,  fcvt.w.s, 0x00,          1,  1.0, rtz);
  TEST_FP_INT_OP_S( 7,  fcvt.w.s, 0x01,          1,  1.1, rtz);
  //TEST_FP_INT_OP_S( 8,  fcvt.w.s, 0x10,     -1<<31, -3e9, rtz);
  //TEST_FP_INT_OP_S( 9,  fcvt.w.s, 0x10,  (1<<31)-1,  3e9, rtz);

  TEST_FP_INT_OP_S(12, fcvt.wu.s, 0x10,          0, -3.0, rtz);
  TEST_FP_INT_OP_S(13, fcvt.wu.s, 0x10,          0, -1.0, rtz);
  TEST_FP_INT_OP_S(14, fcvt.wu.s, 0x01,          0, -0.9, rtz);
  TEST_FP_INT_OP_S(15, fcvt.wu.s, 0x01,          0,  0.9, rtz);
  TEST_FP_INT_OP_S(16, fcvt.wu.s, 0x00,          1,  1.0, rtz);
  TEST_FP_INT_OP_S(17, fcvt.wu.s, 0x01,          1,  1.1, rtz);
  TEST_FP_INT_OP_S(18, fcvt.wu.s, 0x10,          0, -3e9, rtz);
  // TEST_FP_INT_OP_S(19, fcvt.wu.s, 0x00, 3000000000,  3e9, rtz);  //assembler will not generate 3000000000

#if __riscv_xlen >= 64
  TEST_FP_INT_OP_S(22,  fcvt.l.s, 0x01,         -1, -1.1, rtz);
  TEST_FP_INT_OP_S(23,  fcvt.l.s, 0x00,         -1, -1.0, rtz);
  TEST_FP_INT_OP_S(24,  fcvt.l.s, 0x01,          0, -0.9, rtz);
  TEST_FP_INT_OP_S(25,  fcvt.l.s, 0x01,          0,  0.9, rtz);
  TEST_FP_INT_OP_S(26,  fcvt.l.s, 0x00,          1,  1.0, rtz);
  TEST_FP_INT_OP_S(27,  fcvt.l.s, 0x01,          1,  1.1, rtz);

  TEST_FP_INT_OP_S(32, fcvt.lu.s, 0x10,          0, -3.0, rtz);
  TEST_FP_INT_OP_S(33, fcvt.lu.s, 0x10,          0, -1.0, rtz);
  TEST_FP_INT_OP_S(34, fcvt.lu.s, 0x01,          0, -0.9, rtz);
  TEST_FP_INT_OP_S(35, fcvt.lu.s, 0x01,          0,  0.9, rtz);
  TEST_FP_INT_OP_S(36, fcvt.lu.s, 0x00,          1,  1.0, rtz);
  TEST_FP_INT_OP_S(37, fcvt.lu.s, 0x01,          1,  1.1, rtz);
  TEST_FP_INT_OP_S(38, fcvt.lu.s, 0x10,          0, -3e9, rtz);
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

//#define VAL1 -1<<31
//#define VAL2 (1<<31)-1
asm(".data");
RVTEST_DATA_BEGIN
  TEST_INT_FP_OP_DATA1( 2,        -1, -1.1);
  TEST_INT_FP_OP_DATA1( 3,        -1, -1.0);
  TEST_INT_FP_OP_DATA1( 4,         0, -0.9);
  TEST_INT_FP_OP_DATA1( 5,         0,  0.9);
  TEST_INT_FP_OP_DATA1( 6,         1,  1.0);
  TEST_INT_FP_OP_DATA1( 7,         1,  1.1);
  //TEST_INT_FP_OP_DATA1( 8,    VAL1, -3e9);
  //TEST_INT_FP_OP_DATA1( 9,    VAL2,  3e9);

  TEST_INT_FP_OP_DATA1(12,        0, -3.0);
  TEST_INT_FP_OP_DATA1(13,        0, -1.0);
  TEST_INT_FP_OP_DATA1(14,        0, -0.9);
  TEST_INT_FP_OP_DATA1(15,        0,  0.9);
  TEST_INT_FP_OP_DATA1(16,        1,  1.0);
  TEST_INT_FP_OP_DATA1(17,        1,  1.1);
  TEST_INT_FP_OP_DATA1(18,        0, -3e9);
  TEST_INT_FP_OP_DATA1(19, 3000000000,  3e9);

#if __riscv_xlen >= 64
  TEST_INT_FP_OP_DATA1(22,        -1, -1.1);
  TEST_INT_FP_OP_DATA1(23,        -1, -1.0);
  TEST_INT_FP_OP_DATA1(24,         0, -0.9);
  TEST_INT_FP_OP_DATA1(25,         0,  0.9);
  TEST_INT_FP_OP_DATA1(26,         1,  1.0);
  TEST_INT_FP_OP_DATA1(27,         1,  1.1);

  TEST_INT_FP_OP_DATA1(32,        0, -3.0);
  TEST_INT_FP_OP_DATA1(33,        0, -1.0);
  TEST_INT_FP_OP_DATA1(34,        0, -0.9);
  TEST_INT_FP_OP_DATA1(35,        0,  0.9);
  TEST_INT_FP_OP_DATA1(36,        1,  1.0);
  TEST_INT_FP_OP_DATA1(37,        1,  1.1);
  TEST_INT_FP_OP_DATA1(38,        0, -3e9);
#endif

RVTEST_DATA_END
