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

#include "stdlib.h"
#include "stdint.h"
#include "rev-macros.h"
#include "syscalls.h"

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

volatile unsigned thread1_counter = 0;
volatile unsigned thread2_counter = 0;

void *thread1() {
  TRACE_PUSH_ON
  for (int i=0;i<10;i++) thread1_counter++;
  TRACE_PUSH_OFF
  return 0;
}

void *thread2() {
  TRACE_PUSH_ON
  for (int i=0;i<10;i++) thread2_counter+=2;
  TRACE_PUSH_OFF
  return 0;
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

  TRACE_ON;

   // Differentiate rv32i and rv64g operations
 check_mem_access_sizes:
   volatile size_t dst = 0;
   asm volatile("addi t3, zero, -1   \n\t"
 	        "sb   t3, 0(%0)      \n\t"
		"lb   t4, 0(%0)      \n\t"
 	        "sh   t3, 0(%0)      \n\t"
 	        "lh   t4, 0(%0)      \n\t"
 	        "sw   t3, 0(%0)      \n\t"
 	        "lw   t4, 0(%0)      \n\t"
 	        : : "r"(&dst) : "t3", "t4"
 	        );

#ifdef RV64G
   asm volatile("sd   t3, 0(%0)      \n\t"
	        "lwu  t4, 0(%0)      \n\t"
	        "ld   t4, 0(%0)      \n\t"
	        : : "r"(&dst) : "t3", "t4"
	        );
#endif
   
  // trace some memory operations with register dependencies
  // in a tight loop
 check_tight_loop:
  volatile uint32_t load_data = 0x1ace4fee;
  asm volatile("addi t3, zero, 3 \n\t"  // counter = 3
	       "mem_loop:        \n\t"
	       "lw   t4, 0(%0)   \n\t"
	       "addi t3, t3, -1  \n\t"  // counter--
	       "addi t4, t4, 1   \n\t"  // stall?
	       "sw   t4, 0(%0)   \n\t"  // more traffic
	       "bnez t3, mem_loop"          
	       :  : "r"(&load_data) : "t3", "t4"
	       );

  // trace some threads
#if 0  
  rev_pthread_t tid1, tid2;
  rev_pthread_create(&tid1, NULL, (void *)thread1, NULL);
  rev_pthread_create(&tid2, NULL, (void *)thread2, NULL);
  rev_pthread_join(tid1);
  rev_pthread_join(tid2);
  
  TRACE_ASSERT(thread1_counter==10);
  TRACE_ASSERT(thread2_counter==20);
#endif  
  
  return 0;
}
