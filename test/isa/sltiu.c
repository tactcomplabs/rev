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

 // #-------------------------------------------------------------
 // # Arithmetic tests
 // #-------------------------------------------------------------

  TEST_IMM_OP( 2,  sltiu, 0, 0x0000000000000000, 0x000 );
  TEST_IMM_OP( 3,  sltiu, 0, 0x0000000000000001, 0x001 );
  TEST_IMM_OP( 4,  sltiu, 1, 0x0000000000000003, 0x007 );
  TEST_IMM_OP( 5,  sltiu, 0, 0x0000000000000007, 0x003 );

  TEST_IMM_OP( 6,  sltiu, 1, 0x0000000000000000, 0x800 );
  TEST_IMM_OP( 7,  sltiu, 0, 0xffffffff80000000, 0x000 );
  TEST_IMM_OP( 8,  sltiu, 1, 0xffffffff80000000, 0x800 );

  TEST_IMM_OP( 9,  sltiu, 1, 0x0000000000000000, 0x7ff );
  TEST_IMM_OP( 10, sltiu, 0, 0x000000007fffffff, 0x000 );
  TEST_IMM_OP( 11, sltiu, 0, 0x000000007fffffff, 0x7ff );

  TEST_IMM_OP( 12, sltiu, 0, 0xffffffff80000000, 0x7ff );
  TEST_IMM_OP( 13, sltiu, 1, 0x000000007fffffff, 0x800 );

  TEST_IMM_OP( 14, sltiu, 1, 0x0000000000000000, 0xfff );
  TEST_IMM_OP( 15, sltiu, 0, 0xffffffffffffffff, 0x001 );
  TEST_IMM_OP( 16, sltiu, 0, 0xffffffffffffffff, 0xfff );

  //#-------------------------------------------------------------
  //# Source/Destination tests
  //#-------------------------------------------------------------

  //TEST_IMM_SRC1_EQ_DEST( 17, sltiu, 1, 11, 13 );


asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" ); 
     asm volatile("j continue");

asm volatile("fail:" ); 
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
