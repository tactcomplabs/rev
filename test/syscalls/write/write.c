#include <unistd.h>

int main() {
  const char msg[14] = {"Hello, World!"};
  size_t msg_len = 14; // Length of the message string, including the newline character

  ssize_t bytes_written = write(STDOUT_FILENO, &msg[0], msg_len);
  // if (1){// (bytes_written < 0) {
  //   return 1;
  // }

  return 0;
}
