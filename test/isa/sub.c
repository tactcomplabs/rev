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
#include"isa_test_macros.h"

int main(int argc, char **argv){

 // #-------------------------------------------------------------
 // # Arithmetic tests
 // #-------------------------------------------------------------

TEST_RR_OP( 2,  sub, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000 );
  TEST_RR_OP( 3,  sub, 0x0000000000000000, 0x0000000000000001, 0x0000000000000001 );
  TEST_RR_OP( 4,  sub, 0xfffffffffffffffc, 0x0000000000000003, 0x0000000000000007 );

  TEST_RR_OP( 5,  sub, 0x0000000000008000, 0x0000000000000000, 0xffffffffffff8000 );
  TEST_RR_OP( 6,  sub, 0xffffffff80000000, 0xffffffff80000000, 0x0000000000000000 );
  TEST_RR_OP( 7,  sub, 0xffffffff80008000, 0xffffffff80000000, 0xffffffffffff8000 );

  TEST_RR_OP( 8,  sub, 0xffffffffffff8001, 0x0000000000000000, 0x0000000000007fff );
  TEST_RR_OP( 9,  sub, 0x000000007fffffff, 0x000000007fffffff, 0x0000000000000000 );
  TEST_RR_OP( 10, sub, 0x000000007fff8000, 0x000000007fffffff, 0x0000000000007fff );

  TEST_RR_OP( 11, sub, 0xffffffff7fff8001, 0xffffffff80000000, 0x0000000000007fff );
  TEST_RR_OP( 12, sub, 0x0000000080007fff, 0x000000007fffffff, 0xffffffffffff8000 );

  TEST_RR_OP( 13, sub, 0x0000000000000001, 0x0000000000000000, 0xffffffffffffffff );
  TEST_RR_OP( 14, sub, 0xfffffffffffffffe, 0xffffffffffffffff, 0x0000000000000001 );
  TEST_RR_OP( 15, sub, 0x0000000000000000, 0xffffffffffffffff, 0xffffffffffffffff );

  //#-------------------------------------------------------------
  //# Source/Destination tests
  //#-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 16, sub, 2, 13, 11 );
  TEST_RR_SRC2_EQ_DEST( 17, sub, 3, 14, 11 );
  TEST_RR_SRC12_EQ_DEST( 18, sub, 0, 13 );

int p = 0;
int f = 0;
int n = 0;

char msg[10] = "TEST PASS";
size_t msg_len = 10; // Length of the message string, including the newline character
  
asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" ); 
     asm volatile("ADDI a1, zero, %1" : "=r"(p) :  "I"(10));
     //ssize_t bytes_written = write(STDOUT_FILENO, msg, msg_len);
     asm volatile("j continue");

asm volatile("fail:" ); 
     asm volatile("ADDI a0, zero, %1" : "=r"(f) :  "I"(10));

    if(f > 0){
      const char msg2[5] = "FAIL";
      size_t msg_len2 = 5; // Length of the message string, including the newline character
      //ssize_t bytes_written2 = write(STDOUT_FILENO, msg2, msg_len2);
      return 1;
    }

asm volatile("continue:");
asm volatile("li ra, 0x0");

  return 0;
}
