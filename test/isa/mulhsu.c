/*
 * mulhsu.c
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
  TEST_RR_OP( 2,  mulhsu, 0x00000000, 0x00000000, 0x00000000 );
  TEST_RR_OP( 3,  mulhsu, 0x00000000, 0x00000001, 0x00000001 );
  TEST_RR_OP( 4,  mulhsu, 0x00000000, 0x00000003, 0x00000007 );

  TEST_RR_OP( 5,  mulhsu, 0x0000000000000000, 0x0000000000000000, 0xffffffffffff8000 );
  TEST_RR_OP( 6,  mulhsu, 0x0000000000000000, 0xffffffff80000000, 0x00000000 );
  TEST_RR_OP( 7,  mulhsu, 0xffffffff80000000, 0xffffffff80000000, 0xffffffffffff8000 );

 // #-------------------------------------------------------------
 // # Source/Destination tests
 // #-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 8, mulhsu, 143, 13<<32, 11<<32 );
  TEST_RR_SRC2_EQ_DEST( 9, mulhsu, 154, 14<<32, 11<<32 );
  TEST_RR_SRC12_EQ_DEST( 10, mulhsu, 169, 13<<32 );

  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
