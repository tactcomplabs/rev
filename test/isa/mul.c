/*
 * mull.c
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

  TEST_RR_OP(32,  mul, 0x0000000000001200, 0x0000000000007e00, 0x6db6db6db6db6db7 );
  TEST_RR_OP(33,  mul, 0x0000000000001240, 0x0000000000007fc0, 0x6db6db6db6db6db7 );

  TEST_RR_OP( 2,  mul, 0x00000000, 0x00000000, 0x00000000 );
  TEST_RR_OP( 3,  mul, 0x00000001, 0x00000001, 0x00000001 );
  TEST_RR_OP( 4,  mul, 0x00000015, 0x00000003, 0x00000007 );

  TEST_RR_OP( 5,  mul, 0x0000000000000000, 0x0000000000000000, 0xffffffffffff8000 );
  TEST_RR_OP( 6,  mul, 0x0000000000000000, 0xffffffff80000000, 0x00000000 );
  TEST_RR_OP( 7,  mul, 0x0000400000000000, 0xffffffff80000000, 0xffffffffffff8000 );

  TEST_RR_OP(30,  mul, 0x000000000000ff7f, 0xaaaaaaaaaaaaaaab, 0x000000000002fe7d );
  TEST_RR_OP(31,  mul, 0x000000000000ff7f, 0x000000000002fe7d, 0xaaaaaaaaaaaaaaab );

  //#-------------------------------------------------------------
  //# Source/Destination tests
  //#-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 8, mul, 143, 13, 11 );
  TEST_RR_SRC2_EQ_DEST( 9, mul, 154, 14, 11 );
  TEST_RR_SRC12_EQ_DEST( 10, mul, 169, 13 );


  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
