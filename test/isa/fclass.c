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

  TEST_FCLASS_S( 2, 1 << 0, 0xff800000 )
  TEST_FCLASS_S( 3, 1 << 1, 0xbf800000 )
  TEST_FCLASS_S( 4, 1 << 2, 0x807fffff )
  TEST_FCLASS_S( 5, 1 << 3, 0x80000000 )
  TEST_FCLASS_S( 6, 1 << 4, 0x00000000 )
  TEST_FCLASS_S( 7, 1 << 5, 0x007fffff )
  TEST_FCLASS_S( 8, 1 << 6, 0x3f800000 )
  TEST_FCLASS_S( 9, 1 << 7, 0x7f800000 )
  TEST_FCLASS_S(10, 1 << 8, 0x7f800001 )
  //TEST_FCLASS_S(11, 1 << 9, 0x7fc00000 ) // All NaNs classified as signaling

  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(0);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
