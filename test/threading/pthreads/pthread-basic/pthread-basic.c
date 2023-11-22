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
void *thread3() {
  const char msg[29] = "Howdy from thread3 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}
void *thread4() {
  const char msg[29] = "Howdy from thread4 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}
void *thread5() {
  const char msg[29] = "Howdy from thread5 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}
void *thread6() {
  const char msg[29] = "Howdy from thread6 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}
void *thread7() {
  const char msg[29] = "Howdy from thread7 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}
void *thread8() {
  const char msg[29] = "Howdy from thread8 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  // Convert the number to a string
  return 0;
}

int main(int argc, char **argv) {
  const char first_msg[23] = "Welcome to the circus\n";
  rev_write(STDOUT_FILENO, first_msg, sizeof(first_msg));
  // create the thread objs
  rev_pthread_t tid1, tid2, tid3, tid4, tid5, tid6, tid7, tid8;
  // uint64_t thr = 1;
  // uint64_t thr2 = 2;
  // start the threads
  rev_pthread_create(&tid1, NULL, (void *)thread1, NULL);
  rev_pthread_create(&tid2, NULL, (void *)thread2, NULL);
  rev_pthread_create(&tid3, NULL, (void *)thread3, NULL);
  rev_pthread_create(&tid4, NULL, (void *)thread4, NULL);
  rev_pthread_create(&tid5, NULL, (void *)thread5, NULL);
  rev_pthread_create(&tid6, NULL, (void *)thread6, NULL);
  rev_pthread_create(&tid7, NULL, (void *)thread7, NULL);
  rev_pthread_create(&tid8, NULL, (void *)thread8, NULL);

  const char joined_msg[76] = "thread w/ tid1 has finished and been joined. "
                              "Now proceeding with execution\n";
  rev_pthread_join(tid1);
  rev_write(STDOUT_FILENO, joined_msg, sizeof(joined_msg));
  rev_pthread_join(tid2);
  rev_pthread_join(tid3);
  rev_pthread_join(tid4);
  rev_pthread_join(tid5);
  rev_pthread_join(tid6);
  rev_pthread_join(tid7);
  rev_pthread_join(tid8);

  const char msg[26] = "Bonjour from main thread\n";
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  return 0;
}
