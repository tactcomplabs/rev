#include "../../../../common/syscalls/syscalls.h"
// #include<pthread.h>
// a simple pthread example 
// compile with -lpthreads

// create the function to be executed as a thread
void *thread(void *ptr)
{
    int type = (int) ptr;
    char *msg = "Hello from thread "; 
    rev_write(STDOUT_FILENO,msg, sizeof(msg));
    rev_write(STDOUT_FILENO,(char*)type, sizeof(type));
    return  ptr;
}

int main(int argc, char **argv)
{
    // create the thread objs
    uint32_t tid0, tid1;
    int thr = 1;
    int thr2 = 2;
    // start the threads
    rev_pthread_create(*thread);
    rev_pthread_create(*thread);
    // wait for threads to finish
    // pthread_join(thread1,NULL);
    // pthread_join(thread2,NULL);
   rev_exit(99);
    return 0;
}