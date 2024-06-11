#include <string.h>

int main( int argc, char** argv ) {
  if( argc != 3 || !argv[1][0] || !argv[2][0] || strcmp( argv[1], argv[2] ) || (size_t) &argv[2] - (size_t) &argv[1] != sizeof( void* ) )
    asm( " .word 0" );
}
