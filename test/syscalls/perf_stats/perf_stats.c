#include "../../../common/syscalls/syscalls.h"
#include <stdio.h>
#include <string.h>

void print( const char* prefix, unsigned long x ) {
  char buf[256];
  sprintf( buf, "%s%lu\n", prefix, x );
  rev_write( STDOUT_FILENO, buf, strlen( buf ) );
}

int main( int argc, char* argv[] ) {
  int              ret = 0;
  struct rev_stats rs1, rs2;
  rev_perf_stats( &rs1 );
  for( int i = 0; i < 10 * argc; i++ )
    ret++;
  rev_perf_stats( &rs2 );

  print( "instructions: ", rs2.instructions - rs1.instructions );
  print( "cycles: ", rs2.cycles - rs1.cycles );
  return ret;
}
