//clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef HOST_TARGET
#include "revio.h"
#endif
//clang-format on

int main() {

  const char msg[]         = "Greetings\n";
  ssize_t    bytes_written = write( STDOUT_FILENO, msg, sizeof( msg ) - 1 );

  if( bytes_written < 0 )
    exit( 1 );

  const char msg2[]         = "Greetings - this is a much longer text string. Just larger than 64\n";
  ssize_t    bytes_written2 = write( STDOUT_FILENO, msg2, sizeof( msg2 ) - 1 );

  if( bytes_written2 < 0 )
    exit( 2 );

  printf( "Test: %s\n", msg2 );

  int i = 42;
  printf( "The meaning of life is %d\n", i );

  //The test below fails - we are reaching into invalid address space, this appears unrealted to most recent changes
  const char msg3[]         = "Greetings - this is a much longer message and some nice text, in fact, it is bigger than 64 bytes\n";
  ssize_t    bytes_written3 = write( STDOUT_FILENO, msg3, sizeof( msg3 ) - 1 );

  if( bytes_written3 < 0 )
    exit( 3 );

  return 0;
}
