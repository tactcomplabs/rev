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


  float a = 0.0f;
  float b = 0.0f;
  float q = 0.0f;
  float x = 0.0f;

  a = 2.5f;
  b = 1.0f;
  q = a + b;
  x = 3.5f;
  assert( x == q );

  a = 2.5f;
  b = 1.0f;
  q = a - b;
  assert( 1.5f == q );

  a = 2.5f;
  b = 1.0f;
  q = a * b;
  assert( 2.5f == q );

  return 0;
}
