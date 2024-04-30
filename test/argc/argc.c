/*
 * argc.c
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>

#define assert( x )               \
  do                              \
    if( !( x ) ) {                \
      asm( ".dword 0x00000000" ); \
    }                             \
  while( 0 )

// called with "argc.exe one two three; so argc == 4"
int main( int argc, char** argv ) {
  int a = argc;
  assert( a == 4 );
  assert( argv[0][0] == 'a' );
  assert( argv[0][1] == 'r' );
  assert( argv[0][2] == 'g' );
  assert( argv[0][3] == 'c' );
  assert( argv[0][4] == '.' );
  assert( argv[0][5] == 'e' );
  assert( argv[0][6] == 'x' );
  assert( argv[0][7] == 'e' );
  assert( argv[1][0] == 'o' );
  assert( argv[1][1] == 'n' );
  assert( argv[1][2] == 'e' );
  assert( argv[2][0] == 't' );
  assert( argv[2][1] == 'w' );
  assert( argv[2][2] == 'o' );
  assert( argv[3][0] == 't' );
  assert( argv[3][1] == 'h' );
  assert( argv[3][2] == 'r' );
  assert( argv[3][3] == 'e' );
  assert( argv[3][4] == 'e' );
  return 0;
}
