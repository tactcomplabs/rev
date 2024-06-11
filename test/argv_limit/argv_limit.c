/*
 * argv_limit.c
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>

int main( int argc, char** argv ) {
  unsigned long sum = 0;
  if( argc != 4097 )
    asm( " .word 0" );
  for( int i = 1; i < argc; ++i )
    sum += strtoul( argv[i], NULL, 0 );
  if( sum != 8390656 )
    asm( " .word 0" );
  return 0;
}
