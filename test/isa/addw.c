/*
 * addw.c
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

  TEST_RR_OP( 2,  addw, 0x00000000, 0x00000000, 0x00000000 );
  TEST_RR_OP( 3,  addw, 0x00000002, 0x00000001, 0x00000001 );
  TEST_RR_OP( 4,  addw, 0x0000000a, 0x00000003, 0x00000007 );

  TEST_RR_OP( 5,  addw, 0xffffffffffff8000, 0x0000000000000000, 0xffffffffffff8000 );
  TEST_RR_OP( 6,  addw, 0xffffffff80000000, 0xffffffff80000000, 0x00000000 );
  TEST_RR_OP( 7,  addw, 0x000000007fff8000, 0xffffffff80000000, 0xffffffffffff8000 );

  TEST_RR_OP( 8,  addw, 0x0000000000007fff, 0x0000000000000000, 0x0000000000007fff );
  TEST_RR_OP( 9,  addw, 0x000000007fffffff, 0x000000007fffffff, 0x0000000000000000 );
  TEST_RR_OP( 10, addw, 0xffffffff80007ffe, 0x000000007fffffff, 0x0000000000007fff );

  TEST_RR_OP( 11, addw, 0xffffffff80007fff, 0xffffffff80000000, 0x0000000000007fff );
  TEST_RR_OP( 12, addw, 0x000000007fff7fff, 0x000000007fffffff, 0xffffffffffff8000 );

  TEST_RR_OP( 13, addw, 0xffffffffffffffff, 0x0000000000000000, 0xffffffffffffffff );
  TEST_RR_OP( 14, addw, 0x0000000000000000, 0xffffffffffffffff, 0x0000000000000001 );
  TEST_RR_OP( 15, addw, 0xfffffffffffffffe, 0xffffffffffffffff, 0xffffffffffffffff );

  TEST_RR_OP( 16, addw, 0xffffffff80000000, 0x0000000000000001, 0x000000007fffffff );

  //-------------------------------------------------------------
  // Source/Destination tests
  //-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 17, addw, 24, 13, 11 );
  TEST_RR_SRC2_EQ_DEST( 18, addw, 25, 14, 11 );
  TEST_RR_SRC12_EQ_DEST( 19, addw, 26, 13 );

asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
      assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
