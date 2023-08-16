/*
 * ex1.c
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

volatile int fail(int delay) {
  // how do I fail a test?
  // this will just create an obvious divergence in the trace
  int k=delay;
  int one=1;
  asm volatile("_fail: sub %0,%1,%2\n\t"
                       "bgtz %0, _fail"
	       : "=r"(k) : "r"(k) , "r"(one));
  return k*2;
}

volatile int check(int a) {
  if (a!=42) return 1;
  return 0;
}
      
int main(int argc, char **argv){
  int i = 9;
  i = i + argc;
  asm volatile("xor x0,x0,x0");  // trace off 0x4033
  for (int j=0; j<100000;j++) {
    i+=j;
  }
  asm volatile("xor x0,x0,x0");  // trace on  0x4033

  // specific assembly trace checks
  asm volatile("lui %0, 0xaced1" : "=r" (i)); // load upper immediate

  // load addres emits auipc (add upper immediate to PC) and a load
  void* j = (void*) fail;
  i = i + (unsigned long)j;

  if (check(42) != 0) {
    i+= fail(100);
  }
  
  return i;
}

