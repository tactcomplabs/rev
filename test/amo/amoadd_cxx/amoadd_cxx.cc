#include <atomic>
#define assert( x )               \
  do                              \
    if( !( x ) ) {                \
      asm( ".dword 0x00000000" ); \
    }                             \
  while( 0 )

int main() {
  std::atomic<int> a;
  a = 1;  //amoswap
  assert( a == 1 );
  a++;  //amoadd
  assert( a == 2 );

  std::atomic<unsigned long long> b;
  b = 1;  //amoswap
  assert( b == 1 );
  b++;  //amoadd
  assert( b == 2 );
}
