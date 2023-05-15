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


/*#define TEST_ST_OP( testnum, load_inst, store_inst, result, offset, base ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN(la  x1, base); \
      ASM_GEN(li  x2, result); \
      ASM_GEN(la  x15, 7f);  \
      ASM_GEN(store_inst x2, offset(x1)); \
      ASM_GEN(load_inst x14, offset(x1)); \
      ASM_GEN(j 8f); \
      asm volatile("7:");    \
      ASM_GEN(mv x14, x2); \
      asm volatile("8:");    \
  )*/

#define TEST_ST_OP( testnum, load_inst, store_inst, result, offset, base ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN(la  x1, base); \
      ASM_GEN(li  x2, result); \
      ASM_GEN(store_inst x2, offset(x1)); \
      ASM_GEN(load_inst x14, offset(x1)); \
      ASM_GEN(mv x14, x2); \
  )

int main(int argc, char **argv){

  asm volatile("tdat:");
  asm volatile("tdat1:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat2:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat3:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat4:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat5:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat6:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat7:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat8:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat9:   .dword 0xdeadbeefdeadbeef");
  asm volatile("tdat10:  .dword 0xdeadbeefdeadbeef");



 // #-------------------------------------------------------------
 // # Basic tests
 // #-------------------------------------------------------------
 //
 TEST_ST_OP( 2, ld, sd, 0x00aa00aa00aa00aa, 0,  tdat );


  
  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" ); 
     asm volatile("j continue");

asm volatile("fail:" ); 
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
