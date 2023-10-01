/*
 * lwu.c
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

  TEST_LD_OP( 2, lwu, 0x0000000000ff00ff, 0,  tdat );
  TEST_LD_OP( 3, lwu, 0x00000000ff00ff00, 4,  tdat );
  TEST_LD_OP( 4, lwu, 0x000000000ff00ff0, 8,  tdat );
  TEST_LD_OP( 5, lwu, 0x00000000f00ff00f, 12, tdat );

  // # Test with negative offset

  TEST_LD_OP( 6, lwu, 0x0000000000ff00ff, -12, tdat4 );
  TEST_LD_OP( 7, lwu, 0x00000000ff00ff00, -8,  tdat4 );
  TEST_LD_OP( 8, lwu, 0x000000000ff00ff0, -4,  tdat4 );
  TEST_LD_OP( 9, lwu, 0x00000000f00ff00f, 0,   tdat4 );


  //# Test with a negative base

  TEST_CASE( 10, x5, 0x0000000000ff00ff, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -32); \
    ASM_GEN(lwu x5, 32(x6)); \
  )

  //# Test with unaligned base

  TEST_CASE( 11, x5, 0x00000000ff00ff00, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -3); \
    ASM_GEN(lwu x5, 7(x6)); \
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
  asm ("tdat1:  .word 0x00ff00ff");
  asm ("tdat2:  .word 0xff00ff00");
  asm ("tdat3:  .word 0x0ff00ff0");
  asm ("tdat4:  .word 0xf00ff00f");
RVTEST_DATA_END
