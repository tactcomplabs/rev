/*
 * ex2.c
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

uint64_t A[1024];
uint64_t B[1024];
uint64_t R[1024];

int main( int argc, char** argv ) {
  uint64_t i = 0;
  uint64_t j = 0;
  int      r = 0;

  for( i = 0; i < 512; i++ ) {
    for( unsigned j = 0; j < 512; j++ ) {
      R[j] = A[j] + B[j] * i;
      if( ( R[j] % 2 ) == 0 ) {
        r++;
      }
    }
  }

  return r;
}
