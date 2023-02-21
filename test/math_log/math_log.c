/*
 * math_log.c
 *
 * RISC-V ISA: RV64IMAFDC
 *
 * Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char **argv){
  int num_pe;
  int i;

  num_pe = 4;
  i = (int)(log(num_pe)/log(2));

  while (i != 2) {
  }

  return 0;
}
