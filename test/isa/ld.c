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
 // # Basic tests
 // #-------------------------------------------------------------
 //
  TEST_LD_OP( 2, ld, 0x00ff00ff00ff00ff, 0,  tdat );
  TEST_LD_OP( 3, ld, 0xff00ff00ff00ff00, 8,  tdat );
  TEST_LD_OP( 4, ld, 0x0ff00ff00ff00ff0, 16, tdat );
  TEST_LD_OP( 5, ld, 0xf00ff00ff00ff00f, 24, tdat );

 //  # Test with negative offset

  TEST_LD_OP( 6, ld, 0x00ff00ff00ff00ff, -24, tdat4 );
  TEST_LD_OP( 7, ld, 0xff00ff00ff00ff00, -16, tdat4 );
  TEST_LD_OP( 8, ld, 0x0ff00ff00ff00ff0, -8,  tdat4 );
  TEST_LD_OP( 9, ld, 0xf00ff00ff00ff00f, 0,   tdat4 );

 //  # Test with a negative base

  TEST_CASE( 10, x5, 0x00ff00ff00ff00ff, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -32); \
    ASM_GEN(ld x5, 32(x6)); \
  )

 // # Test with unaligned base

  TEST_CASE( 11, x5, 0xff00ff00ff00ff00, \
    ASM_GEN(la  x6, tdat); \
    ASM_GEN(addi x6, x6, -3); \
    ASM_GEN(ld x5, 11(x6)); \
  )


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

  asm(".data");
RVTEST_DATA_BEGIN
  asm ("tdat:");
  asm ("tdat1:  .dword 0x00ff00ff00ff00ff");
  asm ("tdat2:  .dword 0xff00ff00ff00ff00");
  asm ("tdat3:  .dword 0x0ff00ff00ff00ff0");
  asm ("tdat4:  .dword 0xf00ff00ff00ff00f");
RVTEST_DATA_END
