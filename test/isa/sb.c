/*
 * sb.c
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

  TEST_ST_OP( 2, lb, sb, 0xffffffffffffffaa, 0, tdat );
  TEST_ST_OP( 3, lb, sb, 0x0000000000000000, 1, tdat );
  TEST_ST_OP( 4, lh, sb, 0xffffffffffffefa0, 2, tdat );
  TEST_ST_OP( 5, lb, sb, 0x000000000000000a, 3, tdat );

  // # Test with negative offset

  TEST_ST_OP( 6, lb, sb, 0xffffffffffffffaa, -3, tdat8 );
  TEST_ST_OP( 7, lb, sb, 0x0000000000000000, -2, tdat8 );
  TEST_ST_OP( 8, lb, sb, 0xffffffffffffffa0, -1, tdat8 );
  TEST_ST_OP( 9, lb, sb, 0x000000000000000a, 0,  tdat8 );

 //# Test with a negative base

  TEST_CASE( 10, x5, 0x78, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x12345678); \
    ASM_GEN(addi x10, x6, -32); \
    ASM_GEN(sb x7, 32(x10)); \
    ASM_GEN(lb x5, 0(x6)); \
  )

  //# Test with unaligned base
  TEST_CASE( 11, x5, 0xffffffffffffff98, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x00003098); \
    ASM_GEN(addi x6, x6, -6); \
    ASM_GEN(sb x7, 7(x6)); \
    ASM_GEN(la  x10, tdat10); \
    ASM_GEN(lb x5, 0(x10)); \
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
  asm ("tdat1:   .byte 0xef");
  asm ("tdat2:   .byte 0xef");
  asm ("tdat3:   .byte 0xef");
  asm ("tdat4:   .byte 0xef");
  asm ("tdat5:   .byte 0xef");
  asm ("tdat6:   .byte 0xef");
  asm ("tdat7:   .byte 0xef");
  asm ("tdat8:   .byte 0xef");
  asm ("tdat9:   .byte 0xef");
  asm ("tdat10:  .byte 0xef");
RVTEST_DATA_END
