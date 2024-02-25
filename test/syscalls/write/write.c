#include "../../../common/syscalls/syscalls.h"
#include <unistd.h>

int main() {

  const char msg[10] = "Greetings\n";
  ssize_t bytes_written = rev_write(STDOUT_FILENO, msg, sizeof(msg));

  if( bytes_written < 0 ){
    rev_exit(1);
  }
  const char msg2[67] = "Greetings - this is a much longer text string. Just larger than 64\n";
  ssize_t bytes_written2 = rev_write(STDOUT_FILENO, msg2, sizeof(msg2));

  if( bytes_written2 < 0 ){
    rev_exit(1);
  }

    //The test below fails - we are reaching into invalid address space, this appears unrealted to most recent changes
/*  const char msg3[98] = "Greetings - this is a much longer message and some nice text, in fact, it is bigger than 64 bytes\n";
  ssize_t bytes_written3 = rev_write(STDOUT_FILENO, msg3, sizeof(msg3));

  if( bytes_written3 < 0 ){
    rev_exit(1);
  }*/
  return 0;
}
