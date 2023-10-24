#include "../../../../common/syscalls/syscalls.h"
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

struct ThreadInfo {
  int a;
  int TID;
};

// create the function to be executed as a thread
void *thread1(struct ThreadInfo* info) {
  const char msg[29] = "Hello from thread1 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));
  // Convert the number to a string
  assert(info->a == 1);
  return 0;
}

void *thread2(struct ThreadInfo* info){
  const char msg[29] = "Howdy from thread2 function\n";
  // Append tid to msg
  rev_write(STDOUT_FILENO, msg, sizeof(msg));

  assert(info->a == 2);
  // Convert the number to a string
  return 0;
}

int main(int argc, char **argv) {
    // Create three instances of structs to pass to threads
    struct ThreadInfo thread1_info, thread2_info;
    thread1_info.a = 1;
    thread2_info.a = 2;

    const char first_msg[23] = "Welcome to the circus\n";
    rev_write(STDOUT_FILENO, first_msg, sizeof(first_msg));
    // create the thread objs
    rev_pthread_t tid1, tid2;
    // uint64_t thr = 1;
    // uint64_t thr2 = 2;
    // start the threads
    rev_pthread_create(&tid1, (void*)thread1, &thread1_info);
    rev_pthread_create(&tid2, (void*)thread2, &thread2_info);

    rev_pthread_join(tid1);
    rev_pthread_join(tid2);

    const char msg[26] = "Bonjour from main thread\n";
    rev_write(STDOUT_FILENO, msg, sizeof(msg));
   // rev_exit(99);
    return 0;
}
