#include "../../common/syscalls/syscalls.h"

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

__thread uint64_t tls_var = 42;

void *thread1() {
  const char msg[29] = "Hello from thread1 function\n";
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  assert(tls_var == 42);
  tls_var = 69;
  return 0;
}
void *thread2() {
  const char msg[29] = "Hello from thread1 function\n";
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  assert(tls_var == 42);
  return 0;
}

int main() {
  rev_pthread_t tid1;
  rev_pthread_create(&tid1, NULL, (void *)thread1, NULL);
  rev_pthread_join(tid1);
  return 0;
}
