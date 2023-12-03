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

// uncomment to cause argc bug failure
#define INDUCE_ARGC_BUG

unsigned slow_accumulate(unsigned count, unsigned initial_value);

int main(int argc, char **argv){

  assert(argc==4 || argc==5);

  int arg1 = atoi(argv[1]);
  int arg2 = atoi(argv[2]);
  int arg3 = atoi(argv[3]);
  int arg4 = 0;
  if(argc == 5) arg4 = atoi(argv[4]);

#if 0
  // playing around with small code changes affects the bug
  unsigned v = 0;
  v = slow_accumulate(6,arg1);
  assert(v==70);
  v = slow_accumulate(v,arg2);
  assert(v==74);
  v = slow_accumulate(v,arg3);
  assert(v==76);
  v = slow_accumulate(v,arg4);
  assert(v==76);
#endif
  
#ifndef INDUCE_ARGC_BUG
  assert(arg1==64);
  assert(arg2==4);
  assert(arg3==2);
  assert(arg4==0);
#endif
  
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

