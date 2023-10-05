#include "../../../common/syscalls/syscalls.h"
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

int main() {
  struct rev_cpuinfo info;
  int ret = rev_cpuinfo(&info);
  assert(ret == 0);
  assert(info.cores == 1);
  assert(info.harts_per_core == 1);
  return 0;
}

