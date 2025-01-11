#include <string.h>
#define assert( x )               \
  do                              \
    if( !( x ) ) {                \
      asm( ".dword 0x00000000" ); \
    }                             \
  while( 0 )

#define N ( 1024 )
char mem[N];

int main() {
  memset( mem, 42, N );
  for( int i = 0; i < N; i++ ) {
    assert( mem[i] == 42 );
  }
  memset( mem, 0, N );
  for( int i = 0; i < N; i++ ) {
    assert( mem[i] == 0 );
  }
  return 0;
}
