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

#define TEST_SRLI(n, v, a) \
  TEST_IMM_OP(n, srli, ((v) & ((1 << (__riscv_xlen-1) << 1) - 1)) >> (a), v, a)

  TEST_SRLI( 2,  0xffffffff80000000, 0  );
  TEST_SRLI( 3,  0xffffffff80000000, 1  );
  TEST_SRLI( 4,  0xffffffff80000000, 7  );
  TEST_SRLI( 5,  0xffffffff80000000, 14 );
  TEST_SRLI( 6,  0xffffffff80000001, 31 );

  TEST_SRLI( 7,  0xffffffffffffffff, 0  );
  TEST_SRLI( 8,  0xffffffffffffffff, 1  );
  TEST_SRLI( 9,  0xffffffffffffffff, 7  );
  TEST_SRLI( 10, 0xffffffffffffffff, 14 );
  TEST_SRLI( 11, 0xffffffffffffffff, 31 );

  TEST_SRLI( 12, 0x0000000021212121, 0  );
  TEST_SRLI( 13, 0x0000000021212121, 1  );
  TEST_SRLI( 14, 0x0000000021212121, 7  );
  TEST_SRLI( 15, 0x0000000021212121, 14 );
  TEST_SRLI( 16, 0x0000000021212121, 31 );

 // #-------------------------------------------------------------
//  # Source/Destination tests
 // #-------------------------------------------------------------


asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
