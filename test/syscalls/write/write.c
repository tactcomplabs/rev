#include "../../../common/syscalls.h"
#include <unistd.h>

int main() {

  const char msg[10] = "Greetings"; // {72, 69, 0};
  size_t msg_len = 10;                       // Length of the message string, including the newline character

  ssize_t bytes_written = rev_write(STDOUT_FILENO, &msg, msg_len);

  // if (1){// (bytes_written < 0) {
  //   return 1;
  // }

  return 0;
}
