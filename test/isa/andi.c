/*
 * andi.c
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
 // # Arithmetic tests
 // #-------------------------------------------------------------
  TEST_IMM_OP( 2, andi, 0xff00ff00, 0xff00ff00, 0xf0f );
  TEST_IMM_OP( 3, andi, 0x000000f0, 0x0ff00ff0, 0x0f0 );
  TEST_IMM_OP( 4, andi, 0x0000000f, 0x00ff00ff, 0x70f );
  TEST_IMM_OP( 5, andi, 0x00000000, 0xf00ff00f, 0x0f0 );

 // #-------------------------------------------------------------
 // # Source/Destination tests
 // #-------------------------------------------------------------

  TEST_IMM_SRC1_EQ_DEST( 6, andi, 0x00000000, 0xff00ff00, 0x0f0 );



  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
