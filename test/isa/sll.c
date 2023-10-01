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

  TEST_RR_OP( 2,  sll, 0x0000000000000001, 0x0000000000000001, 0  );
  TEST_RR_OP( 3,  sll, 0x0000000000000002, 0x0000000000000001, 1  );
  TEST_RR_OP( 4,  sll, 0x0000000000000080, 0x0000000000000001, 7  );
  TEST_RR_OP( 5,  sll, 0x0000000000004000, 0x0000000000000001, 14 );
  TEST_RR_OP( 6,  sll, 0x0000000080000000, 0x0000000000000001, 31 );

  TEST_RR_OP( 7,  sll, 0xffffffffffffffff, 0xffffffffffffffff, 0  );
  TEST_RR_OP( 8,  sll, 0xfffffffffffffffe, 0xffffffffffffffff, 1  );
  TEST_RR_OP( 9,  sll, 0xffffffffffffff80, 0xffffffffffffffff, 7  );
  TEST_RR_OP( 10, sll, 0xffffffffffffc000, 0xffffffffffffffff, 14 );
  TEST_RR_OP( 11, sll, 0xffffffff80000000, 0xffffffffffffffff, 31 );

  TEST_RR_OP( 12, sll, 0x0000000021212121, 0x0000000021212121, 0  );
  TEST_RR_OP( 13, sll, 0x0000000042424242, 0x0000000021212121, 1  );
  TEST_RR_OP( 14, sll, 0x0000001090909080, 0x0000000021212121, 7  );
  TEST_RR_OP( 15, sll, 0x0000084848484000, 0x0000000021212121, 14 );
  TEST_RR_OP( 16, sll, 0x1090909080000000, 0x0000000021212121, 31 );

 // # Verify that shifts only use bottom six(rv64) or five(rv32) bits

  TEST_RR_OP( 17, sll, 0x0000000021212121, 0x0000000021212121, 0xffffffffffffffc0 );
  TEST_RR_OP( 18, sll, 0x0000000042424242, 0x0000000021212121, 0xffffffffffffffc1 );
  TEST_RR_OP( 19, sll, 0x0000001090909080, 0x0000000021212121, 0xffffffffffffffc7 );
  TEST_RR_OP( 20, sll, 0x0000084848484000, 0x0000000021212121, 0xffffffffffffffce );

#if __riscv_xlen == 64
  TEST_RR_OP( 21, sll, 0x8000000000000000, 0x0000000021212121, 0xffffffffffffffff );
  TEST_RR_OP( 50, sll, 0x8000000000000000, 0x0000000000000001, 63 );
  TEST_RR_OP( 51, sll, 0xffffff8000000000, 0xffffffffffffffff, 39 );
  TEST_RR_OP( 52, sll, 0x0909080000000000, 0x0000000021212121, 43 );
#endif

 // #-------------------------------------------------------------
 // # Source/Destination tests
 // #-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 22, sll, 0x00000080, 0x00000001, 7  );
  TEST_RR_SRC2_EQ_DEST( 23, sll, 0x00004000, 0x00000001, 14 );
  TEST_RR_SRC12_EQ_DEST( 24, sll, 24, 3 );

asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
