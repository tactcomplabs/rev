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

 // #-------------------------------------------------------------
 // # Arithmetic tests
 // #-------------------------------------------------------------

  TEST_RR_OP( 2,  sltu, 0, 0x00000000, 0x00000000 );
  TEST_RR_OP( 3,  sltu, 0, 0x00000001, 0x00000001 );
  TEST_RR_OP( 4,  sltu, 1, 0x00000003, 0x00000007 );
  TEST_RR_OP( 5,  sltu, 0, 0x00000007, 0x00000003 );

  TEST_RR_OP( 6,  sltu, 1, 0x00000000, 0xffff8000 );
  TEST_RR_OP( 7,  sltu, 0, 0x80000000, 0x00000000 );
  TEST_RR_OP( 8,  sltu, 1, 0x80000000, 0xffff8000 );

  TEST_RR_OP( 9,  sltu, 1, 0x00000000, 0x00007fff );
  TEST_RR_OP( 10, sltu, 0, 0x7fffffff, 0x00000000 );
  TEST_RR_OP( 11, sltu, 0, 0x7fffffff, 0x00007fff );

  TEST_RR_OP( 12, sltu, 0, 0x80000000, 0x00007fff );
  TEST_RR_OP( 13, sltu, 1, 0x7fffffff, 0xffff8000 );

  TEST_RR_OP( 14, sltu, 1, 0x00000000, 0xffffffff );
  TEST_RR_OP( 15, sltu, 0, 0xffffffff, 0x00000001 );
  TEST_RR_OP( 16, sltu, 0, 0xffffffff, 0xffffffff );

//  #-------------------------------------------------------------
//  # Source/Destination tests
//  #-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 17, sltu, 0, 14, 13 );
  TEST_RR_SRC2_EQ_DEST( 18, sltu, 1, 11, 13 );
  TEST_RR_SRC12_EQ_DEST( 19, sltu, 0, 13 );



asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
