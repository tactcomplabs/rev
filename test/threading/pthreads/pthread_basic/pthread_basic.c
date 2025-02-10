#include "../../../../common/syscalls/syscalls.h"

#define assert( x )               \
  do                              \
    if( !( x ) ) {                \
      asm( ".dword 0x00000000" ); \
    }                             \
  while( 0 )

// create the function to be executed as a thread
void* thread1() {
  const char msg[] = "Hello from thread1 function\n";
  asm( " fence" );

  // Append tid to msg
  rev_write( STDOUT_FILENO, msg, sizeof( msg ) - 1 );
  // Convert the number to a string
  return 0;
}

void* thread2() {
  const char msg[] = "Howdy from thread2 function\n";
  // Append tid to msg
  asm( " fence" );
  rev_write( STDOUT_FILENO, msg, sizeof( msg ) - 1 );

  // Convert the number to a string
  return 0;
}

int main( int argc, char** argv ) {
  const char first_msg[] = "Welcome to the circus\n";
  rev_write( STDOUT_FILENO, first_msg, sizeof( first_msg ) - 1 );
  // create the thread objs
  rev_pthread_t tid1, tid2;
  // uint64_t thr = 1;
  // uint64_t thr2 = 2;
  // start the threads
  rev_pthread_create( &tid1, NULL, (void*) thread1, NULL );
  rev_pthread_create( &tid2, NULL, (void*) thread2, NULL );

  const char joined_msg[] = "thread w/ tid1 has finished and been joined. "
                            "Now proceeding with execution\n";
  rev_pthread_join( tid1 );
  rev_write( STDOUT_FILENO, joined_msg, sizeof( joined_msg ) - 1 );
  rev_pthread_join( tid2 );

  const char msg[] = "Bonjour from main thread\n";
  rev_write( STDOUT_FILENO, msg, sizeof( msg ) - 1 );
  return 0;
}
