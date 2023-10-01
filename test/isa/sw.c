/*
 * sw.c
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

  TEST_ST_OP( 2, lw, sw, 0x0000000000aa00aa, 0,  tdat );
  TEST_ST_OP( 3, lw, sw, 0xffffffffaa00aa00, 4,  tdat );
  TEST_ST_OP( 4, lw, sw, 0x000000000aa00aa0, 8,  tdat );
  TEST_ST_OP( 5, lw, sw, 0xffffffffa00aa00a, 12, tdat );

  // # Test with negative offset

  TEST_ST_OP( 6, lw, sw, 0x0000000000aa00aa, -12, tdat8 );
  TEST_ST_OP( 7, lw, sw, 0xffffffffaa00aa00, -8,  tdat8 );
  TEST_ST_OP( 8, lw, sw, 0x000000000aa00aa0, -4,  tdat8 );
  TEST_ST_OP( 9, lw, sw, 0xffffffffa00aa00a, 0,   tdat8 );


 //# Test with a negative base

  TEST_CASE( 10, x5, 0x12345678, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x12345678); \
    ASM_GEN(addi x10, x6, -32); \
    ASM_GEN(sw x7, 32(x10)); \
    ASM_GEN(lw x5, 0(x6)); \
  )

  //# Test with unaligned base

  TEST_CASE( 11, x5, 0x58213098, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x58213098); \
    ASM_GEN(addi x6, x6, -3); \
    ASM_GEN(sw x7, 7(x6)); \
    ASM_GEN(la  x10, tdat10); \
    ASM_GEN(lw x5, 0(x10)); \
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
  asm ("tdat1:   .word 0xdeadbeef");
  asm ("tdat2:   .word 0xdeadbeef");
  asm ("tdat3:   .word 0xdeadbeef");
  asm ("tdat4:   .word 0xdeadbeef");
  asm ("tdat5:   .word 0xdeadbeef");
  asm ("tdat6:   .word 0xdeadbeef");
  asm ("tdat7:   .word 0xdeadbeef");
  asm ("tdat8:   .word 0xdeadbeef");
  asm ("tdat9:   .word 0xdeadbeef");
  asm ("tdat10:  .word 0xdeadbeef");
RVTEST_DATA_END
