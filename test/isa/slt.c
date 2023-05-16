/*
 * slt.c
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

//  #-------------------------------------------------------------
//  # Arithmetic tests
//  #-------------------------------------------------------------

  TEST_RR_OP( 2,  slt, 0, 0x0000000000000000, 0x0000000000000000 );
  TEST_RR_OP( 3,  slt, 0, 0x0000000000000001, 0x0000000000000001 );
  TEST_RR_OP( 4,  slt, 1, 0x0000000000000003, 0x0000000000000007 );
  TEST_RR_OP( 5,  slt, 0, 0x0000000000000007, 0x0000000000000003 );

  TEST_RR_OP( 6,  slt, 0, 0x0000000000000000, 0xffffffffffff8000 );
  TEST_RR_OP( 7,  slt, 1, 0xffffffff80000000, 0x0000000000000000 );
  TEST_RR_OP( 8,  slt, 1, 0xffffffff80000000, 0xffffffffffff8000 );

  TEST_RR_OP( 9,  slt, 1, 0x0000000000000000, 0x0000000000007fff );
  TEST_RR_OP( 10, slt, 0, 0x000000007fffffff, 0x0000000000000000 );
  TEST_RR_OP( 11, slt, 0, 0x000000007fffffff, 0x0000000000007fff );

  TEST_RR_OP( 12, slt, 1, 0xffffffff80000000, 0x0000000000007fff );
  TEST_RR_OP( 13, slt, 0, 0x000000007fffffff, 0xffffffffffff8000 );

  TEST_RR_OP( 14, slt, 0, 0x0000000000000000, 0xffffffffffffffff );
  TEST_RR_OP( 15, slt, 1, 0xffffffffffffffff, 0x0000000000000001 );
  TEST_RR_OP( 16, slt, 0, 0xffffffffffffffff, 0xffffffffffffffff );

  //#-------------------------------------------------------------
  //# Source/Destination tests
  //#-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 17, slt, 0, 14, 13 );
  TEST_RR_SRC2_EQ_DEST( 18, slt, 1, 11, 13 );
  TEST_RR_SRC12_EQ_DEST( 19, slt, 0, 13 );

asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" ); 
     asm volatile("j continue");

asm volatile("fail:" ); 
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
