#include "../../../common/syscalls/syscalls.h"

#define assert(x)                                                              \
  do                                                                           \
    if (!(x)) {                                                                \
      asm(".dword 0x00000000");                                                \
    }                                                                          \
  while (0)

int main() {
  struct rev_cpuinfo info;
  int ret = rev_cpuinfo(&info);
  assert(ret == 0);
  assert(info.cores == 1);
  assert(info.harts_per_core == 1);
  return 0;
}
