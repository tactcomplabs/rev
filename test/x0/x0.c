/*
 * x0.c
 *
 * RISC-V ISA: RV64IMAFDC
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>

#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

int main(int argc, char **argv){

  asm volatile (" li a0, 6; \
                  li a1, 6; \
                  addi zero, a0, 0; \
                  add a0, zero, a0;");

  asm volatile(" bne a0, a1, fail;");
  asm volatile("pass:" );
     asm volatile("j continue");

  asm volatile("fail:" );
     assert(0);

  asm volatile("continue:");
  asm volatile("li ra, 0x0");


  return 0;
}
