#include "../../../../common/syscalls/syscalls.h"
// create the function to be executed as a thread
void *thread1(void *ptr)
{
  const char msg[29] = "Hello from thread1 function\n"; 
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  // Convert the number to a string

  return  ptr;
}

void *thread2(void *ptr)
{
  const char msg[29] = "Howdy from thread2 function\n"; 
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  
  // Convert the number to a string
  return  ptr;
}

int main(int argc, char **argv) {
    const char first_msg[23] = "Welcome to the circus\n"; 
    rev_write(STDOUT_FILENO, first_msg, sizeof(first_msg));
    // create the thread objs
    // uint64_t tid0, tid1;
    // uint64_t thr = 1;
    // uint64_t thr2 = 2;
    // start the threads
    rev_pthread_create((void*)thread1);
    rev_pthread_create((void*)thread2);
    rev_pthread_join(1024, NULL);
    rev_pthread_join(1025, NULL);
    // wait for threads to finish
    // pthread_join(thread1,NULL);
    // pthread_join(thread2,NULL);
    const char msg[26] = "Bonjour from main thread\n"; 
    rev_write(STDOUT_FILENO, msg, sizeof(msg));
   // rev_exit(99);
    return 0;
}

