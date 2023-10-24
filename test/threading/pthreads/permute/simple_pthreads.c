/*
 * simple_pthreads.c
 *
 * RISC-V ISA: RV32I, RV64G
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

volatile unsigned thread1_counter = 0;
volatile unsigned thread2_counter = 0;
volatile unsigned thread3_counter = 0;

void *thread1() {
  for (int i=0;i<10;i++) thread1_counter++;
  return 0;
}

void *thread2() {
  for (int i=0;i<10;i++) thread2_counter+=2;
  return 0;
}

void *thread3() {
  for (int i=0;i<10;i++) thread3_counter+=3;
  return 0;
}

int main(int argc, char **argv){
  TRACE_ON
    rev_pthread_t tid1, tid2, tid3;
  rev_pthread_create(&tid1, NULL, (void *)thread1, NULL);
  rev_pthread_create(&tid2, NULL, (void *)thread2, NULL);
  rev_pthread_create(&tid3, NULL, (void *)thread3, NULL);

  rev_pthread_join(tid1);
  rev_pthread_join(tid2);
  rev_pthread_join(tid3);
  
  TRACE_ASSERT(thread1_counter==10);
  TRACE_ASSERT(thread2_counter==20);
  TRACE_ASSERT(thread3_counter==30);
  
  TRACE_OFF  
  return 0;
}
