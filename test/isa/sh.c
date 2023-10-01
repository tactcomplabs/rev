/*
 * sh.c
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

  TEST_ST_OP( 2, lh, sh, 0x00000000000000aa, 0, tdat );
  TEST_ST_OP( 3, lh, sh, 0xffffffffffffaa00, 2, tdat );
  TEST_ST_OP( 4, lw, sh, 0xffffffffbeef0aa0, 4, tdat );
  TEST_ST_OP( 5, lh, sh, 0xffffffffffffa00a, 6, tdat );

  TEST_ST_OP( 6, lhu, sh, 0x0000000000002345,  0, tdat4 );
  TEST_ST_OP( 7, lwu, sh, 0x0000000023456789, -2, tdat4 );
  TEST_ST_OP( 8, lhu, sh, 0x0000000000001337,  2, tdat4 );
  TEST_ST_OP( 9, ld, sh,  0xBEEF133723456789,  -2, tdat4 );

  // # Test with negative offset

  TEST_ST_OP( 10, lh, sh, 0x00000000000000aa, -6, tdat8 );
  TEST_ST_OP( 11, lh, sh, 0xffffffffffffaa00, -4, tdat8 );
  TEST_ST_OP( 12, lh, sh, 0x0000000000000aa0, -2, tdat8 );
  TEST_ST_OP( 13, lh, sh, 0xffffffffffffa00a, 0,  tdat8 );


 //# Test with a negative base

  TEST_CASE( 14, x5, 0x5678, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x12345678); \
    ASM_GEN(addi x10, x6, -32); \
    ASM_GEN(sh x7, 32(x10)); \
    ASM_GEN(lh x5, 0(x6)); \
  )

  //# Test with unaligned base
  TEST_CASE( 15, x5, 0x3098, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x00003098); \
    ASM_GEN(addi x6, x6, -5); \
    ASM_GEN(sh x7, 7(x6)); \
    ASM_GEN(la  x10, tdat10); \
    ASM_GEN(lh x5, 0(x10)); \
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
  asm ("tdat1:   .half 0xbeef");
  asm ("tdat2:   .half 0xbeef");
  asm ("tdat3:   .half 0xbeef");
  asm ("tdat4:   .half 0xbeef");
  asm ("tdat5:   .half 0xbeef");
  asm ("tdat6:   .half 0xbeef");
  asm ("tdat7:   .half 0xbeef");
  asm ("tdat8:   .half 0xbeef");
  asm ("tdat9:   .half 0xbeef");
  asm ("tdat10:  .half 0xbeef");
RVTEST_DATA_END
