/*
 * bug-fatprint.cpp: Testing various issues for fast printf development
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

// Standard includes
#include <cinttypes>
#include <cstdint>
#include <cstdlib>

// REV includes
#include "rev-macros.h"
#include "syscalls.h"

// REV macros
#undef assert
#define assert TRACE_ASSERT
//rev_fast_print limited to a format string, 6 simple numeric parameters, 1024 char max output
#define printf rev_fast_printf
//#define printf(...)
#define REV_TIME( X )                         \
  do {                                        \
    asm volatile( " rdtime %0" : "=r"( X ) ); \
  } while( 0 )

// Globals
const int xfr_size = 256;  // dma transfer size in dwords
uint64_t  check_data[xfr_size];
uint64_t  sram[64];
uint64_t  dram_src[xfr_size];
uint64_t  dram_dst[xfr_size];

int configure() {
  size_t time1, time2;
  REV_TIME( time1 );
  // Generate source and check data
  for( int i = 0; i < xfr_size; i++ ) {
    uint64_t d    = ( 0xaced << 16 ) | i;
    check_data[i] = d;
    dram_src[i]   = d;
  }
  REV_TIME( time2 );
  return time2 - time1;
}

size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  for( int i = 0; i < xfr_size; i++ )
    dram_dst[i] = dram_src[i];
  REV_TIME( time2 );
  return time2 - time1;
}

size_t check() {
  size_t time1, time2;
  REV_TIME( time1 );
  for( int i = 0; i < xfr_size; i++ ) {
    if( check_data[i] != dram_src[i] ) {
      printf( "Failed: check_data[%d]=0x%lx dram_src[%d]=0x%lx\n", i, check_data[i], i, dram_src[i] );
      assert( false );
    }
    if( check_data[i] != dram_dst[i] ) {
      printf( "Failed: check_data[%d]=0x%lx dram_dst[%d]=0x%lx\n", i, check_data[i], i, dram_dst[i] );
      assert( false );
    }
  }
  REV_TIME( time2 );
  return time2 - time1;
}

int main( int argc, char** argv ) {
  printf( "Starting bug test\n" );
  size_t time_config = 0;
  size_t time_exec   = 0;
  size_t time_check  = 0;

  printf(
    "\ndram_dst=0x%lx\ndram_src=0x%lx\nxfr_size=%d\n",
    reinterpret_cast<uint64_t>( dram_dst ),
    reinterpret_cast<uint64_t>( dram_src ),
    xfr_size
  );

  printf( "Configuring...\n" );
  time_config = configure();
  printf( "Executing...\n" );
  time_exec = theApp();
  printf( "Checking...\n" );
  time_check = check();

  printf( "Results:\n" );
  printf( "cycles: config=%ld, exec=%ld, check=%ld\n", time_config, time_exec, time_check );
  printf( "appTest2 completed normally\n" );
  return 0;
}
