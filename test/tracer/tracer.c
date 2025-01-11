/*
 * tracer.c
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

#include <inttypes.h>

#include "rev-macros.h"
#include "stdint.h"
#include "stdlib.h"
#include "syscalls.h"

#ifdef SPIKE
// #include "stdio.h"
#define printf( ... )
#else
#define printf rev_fast_printf
#endif

#define assert TRACE_ASSERT

#if 1
#define REV_TIME( X )                         \
  do {                                        \
    asm volatile( " rdtime %0" : "=r"( X ) ); \
  } while( 0 )
#else
#define REV_TIME( X ) \
  do {                \
    X = 0;            \
  } while( 0 )
#endif

void trace_vec() {
#ifdef RV64GV
  asm volatile( "vsetivli  x0, 0, e8, m1, ta, ma \n\t" );

#endif
}

// inefficient calculation of r-s
int long_sub( int r, int s ) {
  for( int i = 0; i < s; i++ )
    r--;
  return r;
}

volatile int check_push_on( int x, int y ) {
  // turn tracing on without affecting caller tracing state
  TRACE_PUSH_ON;
  // calculate x^y
  int rc = 1;
  for( int i = 0; i < y; i++ )
    rc *= x;
  TRACE_POP;
  return rc;
}

volatile int check_push_off( int r, int s ) {
  // turn tracing on without affecting caller tracing state
  TRACE_PUSH_OFF;
  int rc = long_sub( r, s );
  TRACE_POP;
  return rc;
}

volatile unsigned thread1_counter = 0;
volatile unsigned thread2_counter = 0;

void* thread1() {
  TRACE_PUSH_ON
  for( int i = 0; i < 10; i++ )
    thread1_counter++;
  TRACE_PUSH_OFF
  return 0;
}

void* thread2() {
  TRACE_PUSH_ON
  for( int i = 0; i < 10; i++ )
    thread2_counter += 2;
  TRACE_PUSH_OFF
  return 0;
}

void check_lw_sign_ext() {
#ifdef RV64G
  volatile uint64_t  mem[2]   = { 0 };
  volatile uint64_t* p        = &mem[0];
  volatile uint64_t  badneg   = 0xbadbeef1badbeef0;
  volatile uint64_t  checkneg = 0xffffffffbadbeef0;
  TRACE_PUSH_ON
lw_neq:
  asm volatile( "ld   t3, 0(%1)   \n\t"  // t3 <- badneg
                "ld   t4, 0(%2)   \n\t"  // t4 <- checkneg
                "sd   t3, 4(%0)   \n\t"  // store badneg to upper half of mem[0]
                "lw   t3, 0(%0)   \n\t"  // load mem[0]
                "beq  t3, t4, 0f  \n\t"  // check
                ".word 0x0 \n\t"         // fail
                "0: \n\t"
                "sd   t3, 0(%0)  \n\t"  // store back into mem
                : "=r"( p )
                : "r"( &badneg ), "r"( &checkneg )
                : "t3", "t4" );
  printf( "neg result is 0x%" PRIx64 "\n", *p );
  //assert(resneg==checkneg);

  volatile uint64_t badpos   = 0x7adbeef17adbeef0;
  volatile uint64_t checkpos = 0x07adbeef0;
  mem[0]                     = 0;
  mem[1]                     = 0;
lw_pos:
  asm volatile( "ld   t3, 0(%1)   \n\t"  // t3 <- badpos
                "ld   t4, 0(%2)   \n\t"  // t4 <- checkpos
                "sd   t3, 4(%0)   \n\t"  // store badpos to upper half of mem[0]
                "lw   t3, 0(%0)   \n\t"  // load mem[0]
                "beq  t3, t4, 0f \n\t"   // check
                ".word 0x0 \n\t"         // fail
                "0: \n\t"
                "sd   t3, 0(%0)  \n\t"  // store back into mem
                : "=r"( p )
                : "r"( &badpos ), "r"( &checkpos )
                : "t3", "t4" );
  printf( "pos result is 0x%" PRIx64 "\n", *p );
  TRACE_PUSH_OFF
#endif
}

int main( int argc, char** argv ) {

  // Enable tracing at start to see return instruction pointer
  TRACE_OFF;

  // fast print check
  printf( "check print 1 param: 0x%05x\n", 0xfab6 );
  printf( "check print 6 params: %d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6 );
  printf( "check print 2 with 2 newlines: here->\nhere->\n" );
  printf( "check back to back print with no new lines " );
  printf( " [no new line] " );
  printf( " [no new line] " );
  printf( " ... new line here->\n" );

  trace_vec();

  int res = 3000;
  res     = long_sub( res, 1000 );
  // res == 2000;

  printf( "Enable Tracing\n" );
  // enable tracing
  TRACE_ON;
  res = long_sub( res, 20 );
  // res == 1980
  assert( res == 1980 );
  TRACE_OFF;
  printf( "Tracing Disabled\n" );

  // not traced
  for( int i = 0; i < 1980 / 2; i++ )
    res = res - 1;

  // assert macro enables tracing temporarily
  assert( res * 2 == 1980 );

  // another delay loop to prove tracing still off
  res = long_sub( res, 1980 / 2 );
  assert( res == 0 );

  // call subroutine that uses push/pop to enable/disable tracing
  res = check_push_on( 10, 5 );
  assert( res == 100000 );

  // call subroutine that will not be traced inside a traced loop.
  // again, the assert will be traced
  for( int r = 10; r < 20; r += 10 ) {
    for( int s = -10; s < 10; s += 10 ) {
      int rc = check_push_off( r, s );
      assert( rc = r - s );
    }
  }

  TRACE_ON;

  // Differentiate rv32i and rv64g operations
check_mem_access_sizes:
  volatile size_t dst = 0;
  asm volatile( "addi t3, zero, -1   \n\t"
                "sb   t3, 0(%0)      \n\t"
                "lb   t4, 0(%0)      \n\t"
                "sh   t3, 0(%0)      \n\t"
                "lh   t4, 0(%0)      \n\t"
                "sw   t3, 0(%0)      \n\t"
                "lw   t4, 0(%0)      \n\t"
                :
                : "r"( &dst )
                : "t3", "t4" );

#ifdef RV64G
  asm volatile( "sd   t3, 0(%0)      \n\t"
                "lwu  t4, 0(%0)      \n\t"
                "ld   t4, 0(%0)      \n\t"
                :
                : "r"( &dst )
                : "t3", "t4" );
#endif

  // trace some memory operations with register dependencies
  // in a tight loop
check_tight_loop:
  volatile uint32_t load_data = 0x1ace4fee;
  asm volatile( "addi t3, zero, 3 \n\t"  // counter = 3
                "mem_loop:        \n\t"
                "lw   t4, 0(%0)   \n\t"
                "addi t3, t3, -1  \n\t"  // counter--
                "addi t4, t4, 1   \n\t"  // stall?
                "sw   t4, 0(%0)   \n\t"  // more traffic
                "bnez t3, mem_loop"
                :
                : "r"( &load_data )
                : "t3", "t4" );

  // trace some threads
#if 1
  printf( "Thread test starting\n" );
  rev_pthread_t tid1, tid2;
  rev_pthread_create( &tid1, NULL, (void*) thread1, NULL );
  rev_pthread_create( &tid2, NULL, (void*) thread2, NULL );
  rev_pthread_join( tid1 );
  rev_pthread_join( tid2 );

  TRACE_ASSERT( thread1_counter == 10 );
  TRACE_ASSERT( thread2_counter == 20 );
  printf( "Thread test finished\n" );
#endif

  TRACE_ON;

#if 1
  // use CSR to get time
  size_t time1, time2;
  REV_TIME( time1 );
  int fubar = long_sub( 100, 20 );
  REV_TIME( time2 );
  assert( fubar == 80 );
  printf( "Time check: %" PRId64 "\n", time2 - time1 );
#endif

  check_lw_sign_ext();

  printf( "tracer test completed normally\n" );
  return 0;
}
