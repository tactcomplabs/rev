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

//  #-------------------------------------------------------------
//  # Arithmetic tests
//  #-------------------------------------------------------------

 TEST_RR_OP( 2,  sra, 0xffffffff80000000, 0xffffffff80000000, 0  );
  TEST_RR_OP( 3,  sra, 0xffffffffc0000000, 0xffffffff80000000, 1  );
  TEST_RR_OP( 4,  sra, 0xffffffffff000000, 0xffffffff80000000, 7  );
  TEST_RR_OP( 5,  sra, 0xfffffffffffe0000, 0xffffffff80000000, 14 );
  TEST_RR_OP( 6,  sra, 0xffffffffffffffff, 0xffffffff80000001, 31 );

  TEST_RR_OP( 7,  sra, 0x000000007fffffff, 0x000000007fffffff, 0  );
  TEST_RR_OP( 8,  sra, 0x000000003fffffff, 0x000000007fffffff, 1  );
  TEST_RR_OP( 9,  sra, 0x0000000000ffffff, 0x000000007fffffff, 7  );
  TEST_RR_OP( 10, sra, 0x000000000001ffff, 0x000000007fffffff, 14 );
  TEST_RR_OP( 11, sra, 0x0000000000000000, 0x000000007fffffff, 31 );

  TEST_RR_OP( 12, sra, 0xffffffff81818181, 0xffffffff81818181, 0  );
  TEST_RR_OP( 13, sra, 0xffffffffc0c0c0c0, 0xffffffff81818181, 1  );
  TEST_RR_OP( 14, sra, 0xffffffffff030303, 0xffffffff81818181, 7  );
  TEST_RR_OP( 15, sra, 0xfffffffffffe0606, 0xffffffff81818181, 14 );
  TEST_RR_OP( 16, sra, 0xffffffffffffffff, 0xffffffff81818181, 31 );

 // # Verify that shifts only use bottom six(rv64) or five(rv32) bits

  TEST_RR_OP( 17, sra, 0xffffffff81818181, 0xffffffff81818181, 0xffffffffffffffc0 );
  TEST_RR_OP( 18, sra, 0xffffffffc0c0c0c0, 0xffffffff81818181, 0xffffffffffffffc1 );
  TEST_RR_OP( 19, sra, 0xffffffffff030303, 0xffffffff81818181, 0xffffffffffffffc7 );
  TEST_RR_OP( 20, sra, 0xfffffffffffe0606, 0xffffffff81818181, 0xffffffffffffffce );
  TEST_RR_OP( 21, sra, 0xffffffffffffffff, 0xffffffff81818181, 0xffffffffffffffff );

//  #-------------------------------------------------------------
//  # Source/Destination tests
//  #-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 22, sra, 0xffffffffff000000, 0xffffffff80000000, 7  );
  TEST_RR_SRC2_EQ_DEST( 23, sra, 0xfffffffffffe0000, 0xffffffff80000000, 14 );
  TEST_RR_SRC12_EQ_DEST( 24, sra, 0, 7 );

asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
