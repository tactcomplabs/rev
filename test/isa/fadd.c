/*
 * fadd.c
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
#include "isa_test_macros.h"

int main(int argc, char **argv){

  // #-------------------------------------------------------------
  // # Arithmetic tests
  // #-------------------------------------------------------------


  float a = 0.0;
  float b = 0.0;
  float q = 0.0;
  float x = 0.0;

  a = 2.5;
  b = 1.0;
  q = a + b;
  x = 3.5;

  assert( x == q );

  a = 3.14159265;
  a = -1235.1;
  b = 1.1;
  q = a + b;
  assert( -1234.0 == q );

  a = 3.14159265;
  b = 0.00000001;
  q = a + b;
  assert( 3.1459265 == q );

  a = 2.5;
  b = 1.0;
  q = a - b;
  assert( 1.5 == q );

  a = 3.14159265;
  a = -1235.1;
  b = -1.1;
  q = a - b;
  assert( -1234.0 == q );

  a = 3.14159265;
  b = 0.00000001;
  q = a + b;
  assert( 3.1459265 == q );

  a = 2.5;
  b = 1.0;
  q = a * b;
  assert( 2.5 == q );

  a = 3.14159265;
  a = -1235.1;
  b = -1.1;
  q = a * b;
  assert( 1358.61 == q );

  a = 3.14159265;
  b = 0.00000001;
  q = a * b;
  assert( 3.1459265e-8 == q );


//  TEST_FP_OP2_S( 2,  fadd.s, 0,                3.5,        2.5,        1.0 );
  //TEST_FP_OP2_S( 3,  fadd.s, 1,              -1234,    -1235.1,        1.1 );
  /*TEST_FP_OP2_S( 4,  fadd.s, 1,         3.14159265, 3.14159265, 0.00000001 );

  TEST_FP_OP2_S( 5,  fsub.s, 0,                1.5,        2.5,        1.0 );
  TEST_FP_OP2_S( 6,  fsub.s, 1,              -1234,    -1235.1,       -1.1 );
  TEST_FP_OP2_S( 7,  fsub.s, 1,         3.14159265, 3.14159265, 0.00000001 );

  TEST_FP_OP2_S( 8,  fmul.s, 0,                2.5,        2.5,        1.0 );
  TEST_FP_OP2_S( 9,  fmul.s, 1,            1358.61,    -1235.1,       -1.1 );
  TEST_FP_OP2_S(10,  fmul.s, 1,      3.14159265e-8, 3.14159265, 0.00000001 );*/
  

  
  /*asm volatile(" bne x0, gp, pass;");
asm volatile("pass:" ); 
     asm volatile("j continue");

asm volatile("fail:" ); 
     assert(false);

asm volatile("continue:");
asm volatile("li ra, 0x0");*/

  return 0;
}

