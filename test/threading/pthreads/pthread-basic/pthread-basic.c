#include "../../../../common/syscalls/syscalls.h"
// create the function to be executed as a thread
void *thread1()
{
  const char msg[29] = "Hello from thread1 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  // Convert the number to a string
  return 0;
}

void *thread2()
{
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
    rev_pthread_create(&tid1, (void*)thread1, NULL);
    rev_pthread_create(&tid2, (void*)thread2, NULL);

    rev_pthread_join(tid1);
    rev_pthread_join(tid2);

    const char msg[26] = "Bonjour from main thread\n";
    rev_write(STDOUT_FILENO, msg, sizeof(msg));
   // rev_exit(99);
    return 0;
}
