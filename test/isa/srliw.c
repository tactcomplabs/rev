/*
 * sraiw.c
 *
 * RISC-V ISA: RV64I
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include "isa_test_macros.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

int main( int argc, char** argv ) {

  // #-------------------------------------------------------------
  // # Arithmetic tests
  // #-------------------------------------------------------------
  TEST_IMM_OP( 2, srliw, 0xffffffff80000000, 0xffffffff80000000, 0 );
  TEST_IMM_OP( 3, srliw, 0x0000000040000000, 0xffffffff80000000, 1 );
  TEST_IMM_OP( 4, srliw, 0x0000000001000000, 0xffffffff80000000, 7 );
  TEST_IMM_OP( 5, srliw, 0x0000000000020000, 0xffffffff80000000, 14 );
  TEST_IMM_OP( 6, srliw, 0x0000000000000001, 0xffffffff80000001, 31 );

  TEST_IMM_OP( 7, srliw, 0xffffffffffffffff, 0xffffffffffffffff, 0 );
  TEST_IMM_OP( 8, srliw, 0x000000007fffffff, 0xffffffffffffffff, 1 );
  TEST_IMM_OP( 9, srliw, 0x0000000001ffffff, 0xffffffffffffffff, 7 );
  TEST_IMM_OP( 10, srliw, 0x000000000003ffff, 0xffffffffffffffff, 14 );
  TEST_IMM_OP( 11, srliw, 0x0000000000000001, 0xffffffffffffffff, 31 );

  TEST_IMM_OP( 12, srliw, 0x0000000021212121, 0x0000000021212121, 0 );
  TEST_IMM_OP( 13, srliw, 0x0000000010909090, 0x0000000021212121, 1 );
  TEST_IMM_OP( 14, srliw, 0x0000000000424242, 0x0000000021212121, 7 );
  TEST_IMM_OP( 15, srliw, 0x0000000000008484, 0x0000000021212121, 14 );
  TEST_IMM_OP( 16, srliw, 0x0000000000000000, 0x0000000021212121, 31 );

  //# Verify that shifts ignore top 32 (using true 64-bit values)

  TEST_IMM_OP( 44, srliw, 0x0000000012345678, 0xffffffff12345678, 0 );
  TEST_IMM_OP( 45, srliw, 0x0000000001234567, 0xffffffff12345678, 4 );
  TEST_IMM_OP( 46, srliw, 0xffffffff92345678, 0x0000000092345678, 0 );
  TEST_IMM_OP( 47, srliw, 0x0000000009234567, 0x0000000092345678, 4 );

  //-------------------------------------------------------------
  // Source/Destination tests
  //-------------------------------------------------------------

  // TEST_IMM_SRC1_EQ_DEST( 17, srliw, 0x0000000001000000, 0xffffffff80000000, 7 );

  asm volatile( " bne x0, gp, pass;" );
  asm volatile( "pass:" );
  asm volatile( "j continue" );

  asm volatile( "fail:" );
  assert( false );

  asm volatile( "continue:" );
  asm volatile( "li ra, 0x0" );

  return 0;
}
