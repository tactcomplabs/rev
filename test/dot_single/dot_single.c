/*
 * dot_single.c
 *
 * RISC-V ISA: RV64IMAFDC
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include "cblas.h"
#include <stdlib.h>

float x[1024];
float y[1024];

int main( int argc, char** argv ) {
  float result;
  int   inc_x = 1;
  int   inc_y = 1;
  int   m     = 0;
  int   i     = 0;
  int   l     = 0;

  for( l = 0; l < 1024; l++ ) {
    x[l] = (float) ( l );
    y[l] = (float) ( l ) * (float) ( l );
  }

  for( l = 0; l < 1024; l++ ) {
    result = cblas_sdot( m, &x[0], inc_x, &y[0], inc_y );
  }

  return 0;
}
