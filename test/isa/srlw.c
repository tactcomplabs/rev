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
   TEST_RR_OP( 2,  srlw, 0xffffffff80000000, 0xffffffff80000000, 0  );
   TEST_RR_OP( 3,  srlw, 0x0000000040000000, 0xffffffff80000000, 1  );
   TEST_RR_OP( 4,  srlw, 0x0000000001000000, 0xffffffff80000000, 7  );
   TEST_RR_OP( 5,  srlw, 0x0000000000020000, 0xffffffff80000000, 14 );
   TEST_RR_OP( 6,  srlw, 0x0000000000000001, 0xffffffff80000001, 31 );

   TEST_RR_OP( 7,  srlw, 0xffffffffffffffff, 0xffffffffffffffff, 0  );
   TEST_RR_OP( 8,  srlw, 0x000000007fffffff, 0xffffffffffffffff, 1  );
   TEST_RR_OP( 9,  srlw, 0x0000000001ffffff, 0xffffffffffffffff, 7  );
   TEST_RR_OP( 10, srlw, 0x000000000003ffff, 0xffffffffffffffff, 14 );
   TEST_RR_OP( 11, srlw, 0x0000000000000001, 0xffffffffffffffff, 31 );

   TEST_RR_OP( 12, srlw, 0x0000000021212121, 0x0000000021212121, 0  );
   TEST_RR_OP( 13, srlw, 0x0000000010909090, 0x0000000021212121, 1  );
   TEST_RR_OP( 14, srlw, 0x0000000000424242, 0x0000000021212121, 7  );
   TEST_RR_OP( 15, srlw, 0x0000000000008484, 0x0000000021212121, 14 );
   TEST_RR_OP( 16, srlw, 0x0000000000000000, 0x0000000021212121, 31 );

  //# Verify that shifts only use bottom five bits

  TEST_RR_OP( 17, srlw, 0x0000000021212121, 0x0000000021212121, 0xffffffffffffffe0 );
  TEST_RR_OP( 18, srlw, 0x0000000010909090, 0x0000000021212121, 0xffffffffffffffe1 );
  TEST_RR_OP( 19, srlw, 0x0000000000424242, 0x0000000021212121, 0xffffffffffffffe7 );
  TEST_RR_OP( 20, srlw, 0x0000000000008484, 0x0000000021212121, 0xffffffffffffffee );
  TEST_RR_OP( 21, srlw, 0x0000000000000000, 0x0000000021212121, 0xffffffffffffffff );

  // Verify that shifts ignore top 32 (using true 64-bit values)

  TEST_RR_OP( 44, srlw, 0x0000000012345678, 0xffffffff12345678, 0 );
  TEST_RR_OP( 45, srlw, 0x0000000001234567, 0xffffffff12345678, 4 );
  TEST_RR_OP( 46, srlw, 0xffffffff92345678, 0x0000000092345678, 0 );
  TEST_RR_OP( 47, srlw, 0x0000000009234567, 0x0000000092345678, 4 );
  //-------------------------------------------------------------
  // Source/Destination tests
  //-------------------------------------------------------------


  TEST_RR_SRC1_EQ_DEST( 22, srlw, 0x0000000001000000, 0xffffffff80000000, 7  );
  TEST_RR_SRC2_EQ_DEST( 23, srlw, 0x0000000000020000, 0xffffffff80000000, 14 );
  TEST_RR_SRC12_EQ_DEST( 24, srlw, 0, 7 );


asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
