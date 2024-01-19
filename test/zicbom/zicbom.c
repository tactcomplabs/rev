/*
 * zicbom.c
 *
 * RISC-V ISA: RV64I_Zicbom
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>

#define ARRAY 4096

int A[ARRAY];
int B[ARRAY];
int C[ARRAY];

int main(int argc, char **argv){
  int rtn = 0;
  int i = 0;
  int *APtr = &(A[0]);
  int *BPtr = &B[0];
  int *CPtr = &C[0];

  // init some data
  for( i=0; i<ARRAY; i++ ){
    A[i] = argc+i;
    B[i] = argc*i;
    C[i] = argc << i;
  }

  asm volatile ("cbo.clean (%[addr])": [addr]"+r"(APtr));
  asm volatile ("cbo.flush (%[addr])": [addr]"+r"(BPtr));
  asm volatile ("cbo.inval (%[addr])": [addr]"+r"(CPtr));

  return 0;
}
