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

  TEST_FP_CMP_OP_S( 2, feq.s, 0x00, 1, -1.36, -1.36);
  TEST_FP_CMP_OP_S( 3, fle.s, 0x00, 1, -1.36, -1.36);
  TEST_FP_CMP_OP_S( 4, flt.s, 0x00, 0, -1.36, -1.36);

  TEST_FP_CMP_OP_S( 5, feq.s, 0x00, 0, -1.37, -1.36);
  TEST_FP_CMP_OP_S( 6, fle.s, 0x00, 1, -1.37, -1.36);
  TEST_FP_CMP_OP_S( 7, flt.s, 0x00, 1, -1.37, -1.36);

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
  TEST_FP_OP_DATA2_CMP( 2, 1,  -1.36, -1.36);
  TEST_FP_OP_DATA2_CMP( 3, 1,  -1.36, -1.36);
  TEST_FP_OP_DATA2_CMP( 4, 0, -1.36, -1.36);

  TEST_FP_OP_DATA2_CMP( 5,  0, -1.37, -1.36);
  TEST_FP_OP_DATA2_CMP( 6,  1, -1.37, -1.36);
  TEST_FP_OP_DATA2_CMP( 7,  1, -1.37, -1.36);
RVTEST_DATA_END
