#include "../../../common/syscalls/syscalls.h"
#include "unistd.h"
// #include "sys/mman.h"
#define assert( x )               \
  do                              \
    if( !( x ) ) {                \
      asm( ".dword 0x00000000" ); \
    }                             \
  while( 0 )

int main() {
  uint64_t* addr;
#define N 237

  // Create an anonymous memory mapping
  addr = (uint64_t*) rev_mmap(
    0,  // Let rev choose the address
    N * sizeof( uint64_t ),
    PROT_READ | PROT_WRITE | PROT_EXEC,  // RWX permissions
    MAP_PRIVATE | MAP_ANONYMOUS,         // Not shared, anonymous
    -1,                                  // No file descriptor because it's an anonymous mapping
    0
  );  // No offset, irrelevant for anonymous mappings

  for( uint64_t i = 0; i < N; i++ ) {
    addr[i] = i;
  }
  for( uint64_t i = 0; i < N; i++ ) {
    assert( addr[i] == i );
  }
}
