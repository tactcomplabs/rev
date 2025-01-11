/*
 * backingstore.c
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

#include <stdlib.h>

#define LOOP 4096

int A[LOOP];
int B[LOOP];
int C[LOOP];

int main( int argc, char** argv ) {
  int i = 0;

  for( i = 0; i < LOOP; i++ ) {
    A[i] = i;
    B[i] = i + i;
    C[i] = 0;
  }

  for( i = 0; i < LOOP; i++ ) {
    if( i % 2 == 0 ) {
      C[i] = A[i] + B[i];
    } else {
      C[i] = A[i] * B[i];
    }
  }

  return 0;
}
