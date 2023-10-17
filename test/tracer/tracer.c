/*
 * tracer.c
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
#include <cstdint>
#include "rev-macros.h"

#define assert TRACE_ASSERT

// inefficient calculation of r-s
int long_sub(int r, int s) {
  for (int i=0;i<s;i++)
    r--;
  return r;
}

volatile int check_push_on(int x, int y) {
  // turn tracing on without affecting caller tracing state
  TRACE_PUSH_ON;
  // calculate x^y
  int rc = 1;
  for (int i=0;i<y;i++)
    rc *= x;
  TRACE_POP;
  return rc;
}

volatile int check_push_off(int r, int s) {
  // turn tracing on without affecting caller tracing state
  TRACE_PUSH_OFF;
  int rc = long_sub(r,s);
  TRACE_POP;
  return rc;
}

int main(int argc, char **argv){
  
  // tracing is initially off
  int res=3000;
  res = long_sub(res,1000);
  // res == 2000;

  // enable tracing
  TRACE_ON;
  res = long_sub(res,20);
  // res == 1980
  assert(res==1980);
  TRACE_OFF;

  // not traced
  for (int i=0;i<1980/2;i++)
    res = res - 1;

  // assert macro enables tracing temporarily
  assert(res*2==1980);

  // another delay loop to prove tracing still off
  res = long_sub(res,1980/2);
  assert(res==0);
  
  // call subroutine that uses push/pop to enable/disable tracing
  res = check_push_on(10,5);
  assert(res==100000);

  // call subroutine that will not be traced inside a traced loop.
  // again, the assert will be traced
  for (int r=10;r<20;r+=10) {
    for (int s=-10;s<10;s+=10) {
      int rc = check_push_off(r,s);
      assert(rc=r-s);
    }
  }

  // trace some memory operations with register dependencies
  // in a tight loop
  TRACE_ON;
  volatile uint32_t load_data = 0x1ace4fee;
  asm volatile("addi t3, zero, 5    \n\t"  // counter = 5
	       "mem_loop:       \n\t"
	       "lw   t4, 0(%0)  \n\t"
	       "addi t3, t3, -1 \n\t"       // counter--
	       "addi t4, t4, 1  \n\t"       // stall?
	       "sw   t4, 0(%0)  \n\t"       // more traffic
	       "bnez t3, mem_loop"          
	       :  : "r"(&load_data) : "t3", "t4"
	       );
  TRACE_OFF;
  
  return 0;
}

