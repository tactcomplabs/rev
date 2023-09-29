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

  TEST_FP_OP2_S( 2,  fmin.s, 0,        1.0,        2.5,        1.0 );
  TEST_FP_OP2_S( 3,  fmin.s, 0,    -1235.1,    -1235.1,        1.1 );
  TEST_FP_OP2_S( 4,  fmin.s, 0,    -1235.1,        1.1,    -1235.1 );
  TEST_FP_OP2_S( 5,  fmin.s, 0,    -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP2_S( 6,  fmin.s, 0, 0.00000001, 3.14159265, 0.00000001 );
  TEST_FP_OP2_S( 7,  fmin.s, 0,       -2.0,       -1.0,       -2.0 );

  TEST_FP_OP2_S(12,  fmax.s, 0,        2.5,        2.5,        1.0 );
  TEST_FP_OP2_S(13,  fmax.s, 0,        1.1,    -1235.1,        1.1 );
  TEST_FP_OP2_S(14,  fmax.s, 0,        1.1,        1.1,    -1235.1 );
  TEST_FP_OP2_S(15,  fmax.s, 0,    -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP2_S(16,  fmax.s, 0, 3.14159265, 3.14159265, 0.00000001 );
  TEST_FP_OP2_S(17,  fmax.s, 0,       -1.0,       -1.0,       -2.0 );

 // # -0.0 < +0.0
  //TEST_FP_OP2_S(30,  fmin.s, 0,       -0.0,       -0.0,        0.0 );   // -0 < 0 not recognized by c++, so Rev treats these as identical values
  //TEST_FP_OP2_S(31,  fmin.s, 0,       -0.0,        0.0,       -0.0 );
  //TEST_FP_OP2_S(32,  fmax.s, 0,        0.0,       -0.0,        0.0 );
  //TEST_FP_OP2_S(33,  fmax.s, 0,        0.0,        0.0,       -0.0 );

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
  TEST_FP_OP_DATA2( 2,  1.0,        2.5,        1.0 );
  TEST_FP_OP_DATA2( 3,      -1235.1,    -1235.1,        1.1 );
  TEST_FP_OP_DATA2( 4,      -1235.1,        1.1,    -1235.1 );
  TEST_FP_OP_DATA2( 5,      -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP_DATA2( 6,   0.00000001, 3.14159265, 0.00000001 );
  TEST_FP_OP_DATA2( 7,         -2.0,       -1.0,       -2.0 );

  TEST_FP_OP_DATA2(12,          2.5,        2.5,        1.0 );
  TEST_FP_OP_DATA2(13,          1.1,    -1235.1,        1.1 );
  TEST_FP_OP_DATA2(14,          1.1,        1.1,    -1235.1 );
  TEST_FP_OP_DATA2(15,      -1235.1,        NaN,    -1235.1 );
  TEST_FP_OP_DATA2(16,   3.14159265, 3.14159265, 0.00000001 );
  TEST_FP_OP_DATA2(17,         -1.0,       -1.0,       -2.0 );

 // # -0.0 < +0.0
  TEST_FP_OP_DATA2(30,         -0.0,       -0.0,        0.0 );
  TEST_FP_OP_DATA2(31,         -0.0,        0.0,       -0.0 );
  TEST_FP_OP_DATA2(32,          0.0,       -0.0,        0.0 );
  TEST_FP_OP_DATA2(33,          0.0,        0.0,       -0.0 );
RVTEST_DATA_END
