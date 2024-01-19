/*
 * f_ldst.c
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
 // # Basic tests of flw, fld, fsw, and fsd
 // #-------------------------------------------------------------
 //

  TEST_CASE(2, a0, 0x40000000deadbeef, \
      ASM_GEN(la a1, tdat);    \
      ASM_GEN(flw f1, 4(a1));  \
      ASM_GEN(fsw f1, 20(a1)); \
      ASM_GEN(ld a0, 16(a1));
    );

  TEST_CASE(3, a0, 0x1337d00dbf800000, \
      ASM_GEN(la a1, tdat);    \
      ASM_GEN(flw f1, 0(a1));  \
      ASM_GEN(fsw f1, 24(a1)); \
      ASM_GEN(ld a0, 24(a1))
    );

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
  asm (".word 0xbf800000");
  asm (".word 0x40000000");
  asm (".word 0x40400000");
  asm (".word 0xc0800000");
  asm (".word 0xdeadbeef");
  asm (".word 0xcafebabe");
  asm (".word 0xabad1dea");
  asm (".word 0x1337d00d");
RVTEST_DATA_END
