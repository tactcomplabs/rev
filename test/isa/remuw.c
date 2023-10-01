/*
 * add.c
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

  TEST_RR_OP( 2, remuw,   2,  20,   6 );
  TEST_RR_OP( 3, remuw,   2, -20,   6 );
  TEST_RR_OP( 4, remuw,  20,  20,  -6 );
  TEST_RR_OP( 5, remuw, -20, -20,  -6 );

  TEST_RR_OP( 6, remuw,      0, -1<<31,  1 );
  TEST_RR_OP( 7, remuw, -1<<31, -1<<31, -1 );

  TEST_RR_OP( 8, remuw, -1<<31, -1<<31, 0 );
  TEST_RR_OP( 9, remuw,      1,      1, 0 );
  TEST_RR_OP(10, remuw,      0,      0, 0 );

  asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
