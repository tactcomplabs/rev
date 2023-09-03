#include "../../../common/syscalls/syscalls.h"
#include <unistd.h>

int main() {

  const char msg[10] = "Greetings\n"; 
  ssize_t bytes_written = rev_write(STDOUT_FILENO, msg, sizeof(msg));

  if( bytes_written < 0 ){
    rev_exit(1);
  }
  const char msg2[67] = "Greetings Dave Greetings Dave Greetings Dave 123456789012345678901\n";
  ssize_t bytes_writteni2 = rev_write(STDOUT_FILENO, msg2, sizeof(msg2));

  /*const char msg2[98] = "Greetings - this is a much longer message and some nice text, in fact, it is bigger than 64 bytes\n"; 
  ssize_t bytes_writteni2 = rev_write(STDOUT_FILENO, msg2, sizeof(char)*98);*/
  return 0;
}
