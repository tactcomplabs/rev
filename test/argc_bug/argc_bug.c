/*
 * argc_bug.c
 *
 * RISC-V ISA: RV64G
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 * bug test
 */

#include "stdlib.h"
#include "rev-macros.h"
#define assert TRACE_ASSERT

unsigned slow_accumulate(unsigned count, unsigned initial_value);

int main(int argc, char **argv){

  assert(argc==5);
  assert(argv[1][0]=='6');
  assert(argv[1][1]=='4');
  assert(argv[1][2]==0);
  assert(argv[2][0]=='4');
  assert(argv[2][1]==0);
  assert(argv[3][0]=='2');
  assert(argv[3][1]==0);
  assert(argv[4][0]=='3');
  assert(argv[4][1]==0);
  
  int arg1 = atoi(argv[1]);
  int arg2 = atoi(argv[2]);
  int arg3 = atoi(argv[3]);
  int arg4 = atoi(argv[4]);

  assert(arg1==64);
  assert(arg2==4);
  assert(arg3==2);
  assert(arg4==3);

  // playing around with small code changes affects the bug
  unsigned v = 0;
  v = slow_accumulate(6,arg1);
  assert(v==70);
  v = slow_accumulate(v,arg2);
  assert(v==74);
  v = slow_accumulate(v,arg3);
  assert(v==76);
  v = slow_accumulate(v,arg4);
  assert(v==79);
  
  return 0;
}

unsigned slow_accumulate(unsigned count, unsigned initial_value) {
  int rc;
  asm volatile (
		".rept 25 \n\t"
		"xor x0,x0,x0 \n\t"
		".endr \n\t"
		"add %0, %1, %2 \n\t"
		: "=r" (rc)
		: "r" (count), "r" (initial_value)
		);
  return rc;
}

