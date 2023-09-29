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
   TEST_RR_OP( 2,  sllw, 0x0000000000000001, 0x0000000000000001, 0  );
   TEST_RR_OP( 3,  sllw, 0x0000000000000002, 0x0000000000000001, 1  );
   TEST_RR_OP( 4,  sllw, 0x0000000000000080, 0x0000000000000001, 7  );
   TEST_RR_OP( 5,  sllw, 0x0000000000004000, 0x0000000000000001, 14 );
   TEST_RR_OP( 6,  sllw, 0xffffffff80000000, 0x0000000000000001, 31 );

   TEST_RR_OP( 7,  sllw, 0xffffffffffffffff, 0xffffffffffffffff, 0  );
   TEST_RR_OP( 8,  sllw, 0xfffffffffffffffe, 0xffffffffffffffff, 1  );
   TEST_RR_OP( 9,  sllw, 0xffffffffffffff80, 0xffffffffffffffff, 7  );
   TEST_RR_OP( 10, sllw, 0xffffffffffffc000, 0xffffffffffffffff, 14 );
   TEST_RR_OP( 11, sllw, 0xffffffff80000000, 0xffffffffffffffff, 31 );

   TEST_RR_OP( 12, sllw, 0x0000000021212121, 0x0000000021212121, 0  );
   TEST_RR_OP( 13, sllw, 0x0000000042424242, 0x0000000021212121, 1  );
   TEST_RR_OP( 14, sllw, 0xffffffff90909080, 0x0000000021212121, 7  );
   TEST_RR_OP( 15, sllw, 0x0000000048484000, 0x0000000021212121, 14 );
   TEST_RR_OP( 16, sllw, 0xffffffff80000000, 0x0000000021212121, 31 );

   //       # Verify that shifts only use bottom five bits

   TEST_RR_OP( 17, sllw, 0x0000000021212121, 0x0000000021212121, 0xffffffffffffffe0 );
   TEST_RR_OP( 18, sllw, 0x0000000042424242, 0x0000000021212121, 0xffffffffffffffe1 );
   TEST_RR_OP( 19, sllw, 0xffffffff90909080, 0x0000000021212121, 0xffffffffffffffe7 );
   TEST_RR_OP( 20, sllw, 0x0000000048484000, 0x0000000021212121, 0xffffffffffffffee );
   TEST_RR_OP( 21, sllw, 0xffffffff80000000, 0x0000000021212121, 0xffffffffffffffff );

   //# Verify that shifts ignore top 32 (using true 64-bit values)

   TEST_RR_OP( 44, sllw, 0x0000000012345678, 0xffffffff12345678, 0 );
   TEST_RR_OP( 45, sllw, 0x0000000023456780, 0xffffffff12345678, 4 );
   TEST_RR_OP( 46, sllw, 0xffffffff92345678, 0x0000000092345678, 0 );
   TEST_RR_OP( 47, sllw, 0xffffffff93456780, 0x0000000099345678, 4 );

  //-------------------------------------------------------------
  // Source/Destination tests
  //-------------------------------------------------------------

   TEST_RR_SRC1_EQ_DEST( 22, sllw, 0x00000080, 0x00000001, 7  );
   TEST_RR_SRC2_EQ_DEST( 23, sllw, 0x00004000, 0x00000001, 14 );
   TEST_RR_SRC12_EQ_DEST( 24, sllw, 24, 3 );


asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" );
     asm volatile("j continue");

asm volatile("fail:" );
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
