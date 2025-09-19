#include "../../../common/syscalls/syscalls.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  dump_mem_range( 0x10116, 0x10152 );                                //dump _start
  dump_mem_range_to_file( "full_mem_dump.txt", 0x100e8, 1024 * 9 );  //dump most of the .text segment
  dump_stack_to_file( "stack_dump.txt" );
  dump_stack();
}
