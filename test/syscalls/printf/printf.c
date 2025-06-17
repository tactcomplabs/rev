//clang-format off
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef HOST_TARGET
#include "rev-macros.h"
#include "rev-printf.h"
#undef assert
#define assert TRACE_ASSERT
#endif
//clang-format on

int main() {

  const char msg[]         = "[write]Greetings\n";
  ssize_t    bytes_written = write( STDOUT_FILENO, msg, sizeof( msg ) - 1 );

  if( bytes_written < 0 )
    exit( 1 );

  const char msg2[]         = "[write]Greetings - this is a much longer text string. Just larger than 64\n";
  ssize_t    bytes_written2 = write( STDOUT_FILENO, msg2, sizeof( msg2 ) - 1 );

  if( bytes_written2 < 0 )
    exit( 2 );

  int bytes = 0;
  bytes     = printf( "[printf]Greetings with no formatted strings\n" );
  // assert(bytes==44);
  bytes     = printf( "[printf]Test: %s\n", msg2 );
  // assert(bytes==89);

  int i     = 42;
  printf( "[printf]The meaning of life is %d\n", i );

  const char msg3[] = "[write]Greetings - this is a much longer message and some nice text, in fact, it is bigger than 64 bytes\n";
  ssize_t    bytes_written3 = write( STDOUT_FILENO, msg3, sizeof( msg3 ) - 1 );
  if( bytes_written3 < 0 )
    exit( 3 );

  const char msg4[] = "Greetings once again - this is a much much longer message and some even nicer yet more ambitious text, in "
                      "fact, it is bigger than 128 bytes\n";
  printf( "[printf]%s", msg4 );

  const char shortstring[] = "string";
  printf( "[printf] Multiple strings and data: %s[%d] %s[%d] %s[%d]\n", shortstring, 0, shortstring, 1, shortstring, 2 );

  printf( "[printf] hex(0x%x) float(%4.2f) size_t(%zu)\n", 0xace, 3.1415, sizeof( shortstring ) );

  // sprintf
  char s128[128] = { 0 };

  bytes          = sprintf( s128, "[sprintf] test %d", 1 );
  // assert(bytes==16);
  printf( "[printf] %s\n", s128 );

  // sprintf(s128,"[sprintf] test 2");
  // printf("[printf]%s\n",s128);

  printf( "[printf]Completed normally\n" );

  return 0;
}
