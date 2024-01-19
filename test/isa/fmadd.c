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

  TEST_FP_OP3_S( 2,  fmadd.s, 0,                 3.5,  1.0,        2.5,        1.0 );
  TEST_FP_OP3_S( 3,  fmadd.s, 1,              1236.2, -1.0,    -1235.1,        1.1 );
  TEST_FP_OP3_S( 4,  fmadd.s, 0,               -12.0,  2.0,       -5.0,       -2.0 );

  TEST_FP_OP3_S( 5, fnmadd.s, 0,                -3.5,  1.0,        2.5,        1.0 );
  TEST_FP_OP3_S( 6, fnmadd.s, 1,             -1236.2, -1.0,    -1235.1,        1.1 );
  TEST_FP_OP3_S( 7, fnmadd.s, 0,                12.0,  2.0,       -5.0,       -2.0 );

  TEST_FP_OP3_S( 8,  fmsub.s, 0,                 1.5,  1.0,        2.5,        1.0 );
  TEST_FP_OP3_S( 9,  fmsub.s, 1,                1234, -1.0,    -1235.1,        1.1 );
  TEST_FP_OP3_S(10,  fmsub.s, 0,                -8.0,  2.0,       -5.0,       -2.0 );

  TEST_FP_OP3_S(11, fnmsub.s, 0,                -1.5,  1.0,        2.5,        1.0 );
  TEST_FP_OP3_S(12, fnmsub.s, 1,               -1234, -1.0,    -1235.1,        1.1 );
  TEST_FP_OP3_S(13, fnmsub.s, 0,                 8.0,  2.0,       -5.0,       -2.0 );


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
TEST_FP_OP_DATA3( 2,  3.5,  1.0,        2.5,        1.0 );
TEST_FP_OP_DATA3( 3,  1236.2, -1.0,    -1235.1,        1.1 );
TEST_FP_OP_DATA3( 4, -12.0,  2.0,       -5.0,       -2.0 );
TEST_FP_OP_DATA3( 5, -3.5,  1.0,        2.5,        1.0 );
TEST_FP_OP_DATA3( 6, -1236.2, -1.0,    -1235.1,        1.1 );
TEST_FP_OP_DATA3( 7,  12.0,  2.0,       -5.0,       -2.0 );
TEST_FP_OP_DATA3( 8,  1.5,  1.0,        2.5,        1.0 );
TEST_FP_OP_DATA3( 9,  1234, -1.0,    -1235.1,        1.1 );
TEST_FP_OP_DATA3(10,  -8.0,  2.0,       -5.0,       -2.0 );
TEST_FP_OP_DATA3(11,  -1.5,  1.0,        2.5,        1.0 );
TEST_FP_OP_DATA3(12, -1234, -1.0,    -1235.1,        1.1 );
TEST_FP_OP_DATA3(13, 8.0,  2.0,       -5.0,       -2.0 );
RVTEST_DATA_END
