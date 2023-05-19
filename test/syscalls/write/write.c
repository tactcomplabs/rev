#include "../../../common/syscalls/syscalls.h"
#include <unistd.h>

int main() {

  const char msg[10] = "Greetings\n"; 
  ssize_t bytes_written = rev_write(STDOUT_FILENO, &msg, sizeof(msg));

  if( bytes_written < 0 ){
    rev_exit(1);
  }

  return 0;
}
