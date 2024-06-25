/*
 * argv_stack.c
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

int main( int argc, char** argv ) {
  char a[1024];
  if( a + sizeof( a ) > (char*) argv )
    asm( ".word 0" );
  return 0;
}
