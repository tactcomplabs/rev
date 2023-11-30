#include "../../common/syscalls/syscalls.h"

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  };

__thread uint64_t tls_var = 42;

int main() {
  assert(tls_var == 42);
  return 0;
}
