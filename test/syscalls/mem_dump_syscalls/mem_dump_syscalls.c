#include "../../../common/syscalls/syscalls.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  dump_mem_range( 0x50000, 0x55000 );
  dump_mem_range_to_file( "full_mem_dump.txt", 0, 1024 * 1024 * 10 - 1 );
  dump_stack_to_file( "stack_dump.txt" );
  dump_stack();
}
