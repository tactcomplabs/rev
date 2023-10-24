#include "../../../../common/syscalls/syscalls.h"
#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }
// create the function to be executed as a thread
void *thread1() {
  const char msg[29] = "Hello from thread1 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  // Convert the number to a string
  return 0;
}

void *thread2() {
  const char msg[29] = "Howdy from thread2 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}

int main(int argc, char **argv) {
  const char first_msg[23] = "Welcome to the circus\n";
  rev_write(STDOUT_FILENO, first_msg, sizeof(first_msg));
  // create the thread objs
  rev_pthread_t tid1, tid2;
  // uint64_t thr = 1;
  // uint64_t thr2 = 2;
  // start the threads
  rev_pthread_create(&tid1, NULL, (void *)thread1, NULL);
  rev_pthread_create(&tid2, NULL, (void *)thread2, NULL);

  const char joined_msg[76] = "thread w/ tid1 has finished and been joined. "
                              "Now proceeding with execution\n";
  rev_pthread_join(tid1);
  rev_write(STDOUT_FILENO, joined_msg, sizeof(joined_msg));
  rev_pthread_join(tid2);

  const char msg[26] = "Bonjour from main thread\n";
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  // rev_exit(99);
  return 0;
}
