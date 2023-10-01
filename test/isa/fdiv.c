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


  float a = 0.0;
  float b = 0.0;
  float q = 0.0;
  float x = 0.0;

 /* TEST_FP_OP2_S(2,  fdiv.s, 1, 1.1557273520668288, 3.14159265, 2.71828182 );*/
  a = 3.14159265;
  b = 2.71828182;
  q = a / b;
  x = 1.1557273520668288;

  assert( x == q );

  /*TEST_FP_OP2_S(3,  fdiv.s, 1, -0.9991093838555584,      -1234,     1235.1 );*/
  a = -1234;
  b = 1235.1;
  q = a / b;
  x = -0.9991093838555584;
  assert( x == q );

  /*TEST_FP_OP2_S(4,  fdiv.s, 0,         3.14159265, 3.14159265,        1.0 );*/
  a = 3.14159265;
  b = 1.0;
  q = a / b;
  x = 3.14159265;
  assert( x == q );

  TEST_FP_OP2_S(4,  fdiv.s, 0,         3.14159265, 3.14159265,        1.0 );
  TEST_FP_OP2_S(5,  fdiv.s, 0,         1.1557273520668288, 3.14159265,        2.71828182 );

  TEST_FP_OP1_S(6,  fsqrt.s, 1,         1.7724538498928541, 3.14159265 );
  TEST_FP_OP1_S(7,  fsqrt.s, 0,         100,      10000 );
  TEST_FP_OP1_S(8,  fsqrt.s, 1,         13.076696, 171.0);

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
TEST_FP_OP_DATA2(4, 3.14159265, 3.14159265, 1.0);
TEST_FP_OP_DATA2(5, 1.1557273520668288, 3.14159265, 2.71828182);
TEST_FP_OP_DATA1(6, 1.7724538498928541, 3.14159265 );
TEST_FP_OP_DATA1(7, 100,      10000 );
TEST_FP_OP_DATA1(8, 13.076696, 171.0);
RVTEST_DATA_END
