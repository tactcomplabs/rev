/*
 * lb.c
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
#include <stdbool.h>
#include "isa_test_macros.h"


int main(int argc, char **argv){





 // #-------------------------------------------------------------
 // # Basic tests
 // #-------------------------------------------------------------
 //
  TEST_LD_OP( 2, lb, 0xffffffffffffffff, 0,  tdat );
  TEST_LD_OP( 3, lb, 0x0000000000000000, 1,  tdat );
  TEST_LD_OP( 4, lb, 0xfffffffffffffff0, 2,  tdat );
  TEST_LD_OP( 5, lb, 0x000000000000000f, 3, tdat );

  //# Test with negative offset

  TEST_LD_OP( 6, lb, 0xffffffffffffffff, -3, tdat4 );
  TEST_LD_OP( 7, lb, 0x0000000000000000, -2,  tdat4 );
  TEST_LD_OP( 8, lb, 0xfffffffffffffff0, -1,  tdat4 );
  TEST_LD_OP( 9, lb, 0x000000000000000f, 0,   tdat4 );

  //# Test with a negative base

  TEST_CASE( 10, x5, 0xffffffffffffffff, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -32); \
    ASM_GEN(lb x5, 32(x6)); \
  )

  //# Test with unaligned base

  TEST_CASE( 11, x5, 0x0000000000000000, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -6); \
    ASM_GEN(lb x5, 7(x6)); \
  )


  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}

asm(".data");
RVTEST_DATA_BEGIN
  asm ("tdat:");
  asm ("tdat1:  .byte 0xff");
  asm ("tdat2:  .byte 0x00");
  asm ("tdat3:  .byte 0xf0");
  asm ("tdat4:  .byte 0x0f");
RVTEST_DATA_END
