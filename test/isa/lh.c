/*
 * lh.c
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

  TEST_LD_OP( 2, lh, 0x00000000000000ff, 0,  tdat );
  TEST_LD_OP( 3, lh, 0xffffffffffffff00, 2,  tdat );
  TEST_LD_OP( 4, lh, 0x0000000000000ff0, 4,  tdat );
  TEST_LD_OP( 5, lh, 0xfffffffffffff00f, 6, tdat );

  //# Test with negative offset

  TEST_LD_OP( 6, lh, 0x00000000000000ff, -6,  tdat4 );
  TEST_LD_OP( 7, lh, 0xffffffffffffff00, -4,  tdat4 );
  TEST_LD_OP( 8, lh, 0x0000000000000ff0, -2,  tdat4 );
  TEST_LD_OP( 9, lh, 0xfffffffffffff00f,  0, tdat4 );


  TEST_CASE( 10, x5, 0x00000000000000ff, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -32); \
    ASM_GEN(lh x5, 32(x6)); \
  )

 // # Test with unaligned base - currently fails

  TEST_CASE( 11, x5, 0xffffffffffffff00, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -5); \
    ASM_GEN(lh x5, 7(x6)); \
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
  asm ("tdat1:  .half 0x00ff");
  asm ("tdat2:  .half 0xff00");
  asm ("tdat3:  .half 0x0ff0");
  asm ("tdat4:  .half 0xf00f");
RVTEST_DATA_END
