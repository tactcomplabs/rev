// clang-format off
#include "rev-macros.h"
#include "syscalls.h"

#undef assert
#define assert TRACE_ASSERT

int main() {
  uint64_t v64 = 0x80000000000aced1UL;
  uint64_t c64 = 0xbad1;
  int c32 = 0xbad0;

  asm volatile(
	       "c.addi16sp  sp, -224         \n\t"  // alloc on sp
	       "c.sdsp      %2, 144(sp)      \n\t"  // save v64
	       "c.swsp      zero, 152(sp)    \n\t"  // save 0
	       "slli        zero, zero, 10   \n\t"  // do nothing
	       "c.lwsp      %0, 152(sp)      \n\t"  // read 0 ( failure will be reading from physical address -1 )
	       "c.ldsp      %1, 144(sp)      \n\t"  // read v64
	       "c.addi16sp  sp, 224          \n\t"  // restore sp
	       : "=r"(c32), "=r"(c64)
	       : "r"(v64)
	       );

  assert(c32==0x0);
  assert(c64==0x80000000000aced1UL);

  return 0;
}

