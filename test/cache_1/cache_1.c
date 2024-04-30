/*
 * ex1.c
 *
 * RISC-V ISA: RV32I
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>

int main( int argc, char** argv ) {
  int i = 9;
  int j = 0;
  i     = i + argc;
  for( i = 0; i < 1024; i++ ) {
    if( ( j / 3 ) == 0 ) {
      j += i * 12;
    }
  }
  return i;
}
