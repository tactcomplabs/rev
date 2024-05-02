/*
 * fault1.c
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

#include <stdint.h>
#include <stdlib.h>

#define WIDTH 32768

uint64_t VECT_A[WIDTH];
uint64_t VECT_B[WIDTH];
uint64_t RESULT[WIDTH];

int run_this() {
  uint64_t i = 0x00ull;
  uint64_t r = 0x00ull;

  for( i = 0; i < WIDTH; i++ ) {
    VECT_A[i] = i;
    VECT_B[i] = i * i;
    RESULT[i] = 0x00ull;
  }

  for( i = 0; i < WIDTH; i++ ) {
    RESULT[i] = VECT_A[i] + VECT_B[i];
    r += RESULT[i];
  }

  return r;
}

int main( int argc, char** argv ) {
  return run_this();
}
