#include "../../../common/syscalls/syscalls.h"
#define assert(x) if(!(x)) { asm(".byte 0; .byte 0; .byte 0; .byte 0"); }

int main(int argc, char *argv[]) {
  int sum = 0;
  struct __kernel_timespec s, e;

  int ret = rev_clock_gettime(0, &s);
  assert(ret == 0);

  /*
   * Dummy code that is the subject of measurement.
   * We use argc to discourage compiler from removing
   * dead code.
   */
  for(int i = 0; i < 100 * argc; i++)
    sum++;

  ret = rev_clock_gettime(0, &e);
  assert(ret == 0);
 
  assert((e.tv_nsec - s.tv_nsec) >= 0);
  return sum;
}
