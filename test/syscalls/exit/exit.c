/*
 * exit.c
 *
 * Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#ifndef HOST_TARGET
// REV fast printf and tracing macros
#include "rev-macros.h"
#include "syscalls.h"
#define printf rev_fast_printf
#undef assert
#define assert TRACE_ASSERT
#else
#define TRACE_ON
#define TRACE_OFF
#endif

#define xstr( s )         str( s )
#define str( s )          #s

#define TMP1              "hello.tmp"
#define NON_EXISTENT_FILE "non_existent_file.fubar"
#define DATA_CSV          "data.csv"
#define DATA_BIN          "data.bin"

int main() {

#ifdef HOST_TARGET
  printf( "HOST_TARGET not supported\n" );
  return -1;
#else
  TRACE_ON;
  asm volatile( "li a0, 0" );   // exit code to return
  asm volatile( "li a7, 93" );  // exit ecall number
  asm volatile( "ecall" );
#if 0
  asm volatile ("add x0,x0,x0");
#else
  asm volatile( ".word 0x0" );  // crashes sim if prefetched
#endif
  TRACE_OFF;
#endif
  return 1;
}
