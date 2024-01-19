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

  TEST_IMM_OP( 2,  sraiw, 0xffffffff80000000, 0xffffffff80000000, 0  );
  TEST_IMM_OP( 3,  sraiw, 0xffffffffc0000000, 0xffffffff80000000, 1  );
  TEST_IMM_OP( 4,  sraiw, 0xffffffffff000000, 0xffffffff80000000, 7  );
  TEST_IMM_OP( 5,  sraiw, 0xfffffffffffe0000, 0xffffffff80000000, 14 );
  TEST_IMM_OP( 6,  sraiw, 0xffffffffffffffff, 0xffffffff80000001, 31 );

  TEST_IMM_OP( 7,  sraiw, 0x000000007fffffff, 0x000000007fffffff, 0  );
  TEST_IMM_OP( 8,  sraiw, 0x000000003fffffff, 0x000000007fffffff, 1  );
  TEST_IMM_OP( 9,  sraiw, 0x0000000000ffffff, 0x000000007fffffff, 7  );
  TEST_IMM_OP( 10, sraiw, 0x000000000001ffff, 0x000000007fffffff, 14 );
  TEST_IMM_OP( 11, sraiw, 0x0000000000000000, 0x000000007fffffff, 31 );

  TEST_IMM_OP( 12, sraiw, 0xffffffff81818181, 0xffffffff81818181, 0  );
  TEST_IMM_OP( 13, sraiw, 0xffffffffc0c0c0c0, 0xffffffff81818181, 1  );
  TEST_IMM_OP( 14, sraiw, 0xffffffffff030303, 0xffffffff81818181, 7  );
  TEST_IMM_OP( 15, sraiw, 0xfffffffffffe0606, 0xffffffff81818181, 14 );
  TEST_IMM_OP( 16, sraiw, 0xffffffffffffffff, 0xffffffff81818181, 31 );

 // # Verify that shifts ignore top 32 (using true 64-bit values)

  TEST_IMM_OP( 44, sraiw, 0x0000000012345678, 0xffffffff12345678, 0 );
  TEST_IMM_OP( 45, sraiw, 0x0000000001234567, 0xffffffff12345678, 4 );
  TEST_IMM_OP( 46, sraiw, 0xffffffff92345678, 0x0000000092345678, 0 );
  TEST_IMM_OP( 47, sraiw, 0xfffffffff9234567, 0x0000000092345678, 4 );

  //-------------------------------------------------------------
  // Source/Destination tests
  //-------------------------------------------------------------

  //TEST_IMM_SRC1_EQ_DEST( 17, sraiw, 0xffffffffff000000, 0xffffffff80000000, 7 );

asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
