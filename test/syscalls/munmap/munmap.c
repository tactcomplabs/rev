#include "../../../common/syscalls/syscalls.h"
#include "unistd.h"

int main() {
  uint32_t *addr;

  // Create an anonymous memory mapping
  addr = rev_mmap(NULL,                 // Let rev choose the address
              237 * sizeof(uint32_t),                 
              PROT_READ | PROT_WRITE | PROT_EXEC, // RWX permissions
              MAP_PRIVATE | MAP_ANONYMOUS, // Not shared, anonymous
              -1,                   // No file descriptor because it's an anonymous mapping
              0);                   // No offset, irrelevant for anonymous mappings

  
  for( uint32_t i=0; i<120; i++ ){
    addr[i] = i;
  }
  // rev_write(STDOUT_FILENO, &addr[0], 128*sizeof(int));
  // rev_write(STDOUT_FILENO, '\n', 1);

  rev_munmap(*addr, 120);
  // Should segfault at start of loop
  // for( int i=0; i<513; i++ ){
  //   addr[i] = 0;
  // }

  // rev_write(STDOUT_FILENO, addr, 128 );
  // rev_write(STDOUT_FILENO, '\n', 1);
  rev_exit(0);

  // We can now use the memory region pointed to by 'addr' as RWX memory...

  // Unmap the memory region when we're done with it
  // if (rev_munmap(addr, 4096) == -1) {
  //     return 1;
  // }

  return 0;
}
