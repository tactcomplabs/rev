/*
 * sraiw.c
 *
 * RISC-V ISA: RV64I
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
 TEST_RR_OP( 2,  sraw, 0xffffffff80000000, 0xffffffff80000000, 0  );
 TEST_RR_OP( 3,  sraw, 0xffffffffc0000000, 0xffffffff80000000, 1  );
 TEST_RR_OP( 4,  sraw, 0xffffffffff000000, 0xffffffff80000000, 7  );
 TEST_RR_OP( 5,  sraw, 0xfffffffffffe0000, 0xffffffff80000000, 14 );
 TEST_RR_OP( 6,  sraw, 0xffffffffffffffff, 0xffffffff80000001, 31 );

 TEST_RR_OP( 7,  sraw, 0x000000007fffffff, 0x000000007fffffff, 0  );
 TEST_RR_OP( 8,  sraw, 0x000000003fffffff, 0x000000007fffffff, 1  );
 TEST_RR_OP( 9,  sraw, 0x0000000000ffffff, 0x000000007fffffff, 7  );
 TEST_RR_OP( 10, sraw, 0x000000000001ffff, 0x000000007fffffff, 14 );
 TEST_RR_OP( 11, sraw, 0x0000000000000000, 0x000000007fffffff, 31 );

 TEST_RR_OP( 12, sraw, 0xffffffff81818181, 0xffffffff81818181, 0  );
 TEST_RR_OP( 13, sraw, 0xffffffffc0c0c0c0, 0xffffffff81818181, 1  );
 TEST_RR_OP( 14, sraw, 0xffffffffff030303, 0xffffffff81818181, 7  );
 TEST_RR_OP( 15, sraw, 0xfffffffffffe0606, 0xffffffff81818181, 14 );
 TEST_RR_OP( 16, sraw, 0xffffffffffffffff, 0xffffffff81818181, 31 );

 //# Verify that shifts only use bottom five bits

 TEST_RR_OP( 17, sraw, 0xffffffff81818181, 0xffffffff81818181, 0xffffffffffffffe0 );
 TEST_RR_OP( 18, sraw, 0xffffffffc0c0c0c0, 0xffffffff81818181, 0xffffffffffffffe1 );
 TEST_RR_OP( 19, sraw, 0xffffffffff030303, 0xffffffff81818181, 0xffffffffffffffe7 );
 TEST_RR_OP( 20, sraw, 0xfffffffffffe0606, 0xffffffff81818181, 0xffffffffffffffee );
 TEST_RR_OP( 21, sraw, 0xffffffffffffffff, 0xffffffff81818181, 0xffffffffffffffff );

 //# Verify that shifts ignore top 32 (using true 64-bit values)

 TEST_RR_OP( 44, sraw, 0x0000000012345678, 0xffffffff12345678, 0 );
 TEST_RR_OP( 45, sraw, 0x0000000001234567, 0xffffffff12345678, 4 );
 TEST_RR_OP( 46, sraw, 0xffffffff92345678, 0x0000000092345678, 0 );
 TEST_RR_OP( 47, sraw, 0xfffffffff9234567, 0x0000000092345678, 4 );

  //-------------------------------------------------------------
  // Source/Destination tests
  //-------------------------------------------------------------


asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
