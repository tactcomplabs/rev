/*
 * sd.c
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
  TEST_ST_OP( 2, ld, sd, 0x00aa00aa00aa00aa, 0,  tdat );
  TEST_ST_OP( 3, ld, sd, 0xaa00aa00aa00aa00, 8,  tdat );
  TEST_ST_OP( 4, ld, sd, 0x0aa00aa00aa00aa0, 16,  tdat );
  TEST_ST_OP( 5, ld, sd, 0xa00aa00aa00aa00a, 24, tdat );

  //# Test with negative offset

  TEST_ST_OP( 6, ld, sd, 0x00aa00aa00aa00aa, -24, tdat8 );
  TEST_ST_OP( 7, ld, sd, 0xaa00aa00aa00aa00, -16, tdat8 );
  TEST_ST_OP( 8, ld, sd, 0x0aa00aa00aa00aa0, -8,  tdat8 );
  TEST_ST_OP( 9, ld, sd, 0xa00aa00aa00aa00a, 0,   tdat8 );

 //# Test with a negative base

  TEST_CASE( 10, x5, 0x1234567812345678, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x1234567812345678); \
    ASM_GEN(addi x10, x6, -32); \
    ASM_GEN(sd x7, 32(x10)); \
    ASM_GEN(ld x5, 0(x6)); \
  )

  //# Test with unaligned base

  TEST_CASE( 11, x5, 0x5821309858213098, \
    ASM_GEN(la  x6, tdat9); \
    ASM_GEN(li  x7, 0x5821309858213098); \
    ASM_GEN(addi x6, x6, -3); \
    ASM_GEN(sd x7, 11(x6)); \
    ASM_GEN(la  x10, tdat10); \
    ASM_GEN(ld x5, 0(x10)); \
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
  asm ("tdat1:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat2:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat3:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat4:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat5:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat6:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat7:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat8:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat9:   .dword 0xdeadbeefdeadbeef");
  asm ("tdat10:  .dword 0xdeadbeefdeadbeef");
RVTEST_DATA_END
