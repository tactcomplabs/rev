#include "../../../common/syscalls/syscalls.h"
#define assert(x) if(!(x)) { asm(".byte 0; .byte 0; .byte 0; .byte 0"); }

int main() {
  struct __kernel_timespec s, e;

  int ret = rev_clock_gettime(0, &s);
  assert(ret == 0);

  for(int i = 0; i < 100; i++);

  ret = rev_clock_gettime(0, &e);
  assert(ret == 0);
 
  assert((e.tv_nsec - s.tv_nsec) >= 0);
  return 0;
}
