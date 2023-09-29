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
#define TEST_FSGNJS(n, insn, new_sign, rs1_sign, rs2_sign) \
  TEST_CASE(n, a0, 0x12345678 | (-(new_sign) << 31), \
    ASM_GEN( li a1, ((rs1_sign) << 31) | 0x12345678); \
    ASM_GEN(li a2, -(rs2_sign)); \
    ASM_GEN(fmv.s.x f1, a1); \
    ASM_GEN(fmv.s.x f2, a2); \
    ASM_GEN(insn f0, f1, f2); \
    ASM_GEN(fmv.x.s a0, f0); \
  );

  TEST_FSGNJS(10, fsgnj.s, 0, 0, 0)
  TEST_FSGNJS(11, fsgnj.s, 1, 0, 1)
  TEST_FSGNJS(12, fsgnj.s, 0, 1, 0)
  TEST_FSGNJS(13, fsgnj.s, 1, 1, 1)

  TEST_FSGNJS(20, fsgnjn.s, 1, 0, 0)
  TEST_FSGNJS(21, fsgnjn.s, 0, 0, 1)
  TEST_FSGNJS(22, fsgnjn.s, 1, 1, 0)
  TEST_FSGNJS(23, fsgnjn.s, 0, 1, 1)

  TEST_FSGNJS(30, fsgnjx.s, 0, 0, 0)
  TEST_FSGNJS(31, fsgnjx.s, 1, 0, 1)
  TEST_FSGNJS(32, fsgnjx.s, 1, 1, 0)
  TEST_FSGNJS(33, fsgnjx.s, 0, 1, 1)


  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(0);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
