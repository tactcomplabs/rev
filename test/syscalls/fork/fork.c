#include <stdlib.h>
#include <stdio.h>

#include "../../../common/syscalls/syscalls.h"

int main() {
  int pid;

  pid = rev_fork();

  if (pid < 0) {
    rev_exit(pid);
  }

  if (pid == 0) { // This is the child process
    const char msg[27] = "Greetings from the child!\n";// {72, 69, 0};
    size_t msg_len = 27; // Length of the message string, including the newline character
    ssize_t bytes_written = rev_write(0, &msg, msg_len);
    rev_exit(0);
  }
  else {
  // This is the parent process
    const char parent_msg[28] = "Greetings from the parent!\n";// {72, 69, 0};
    size_t parent_msg_len = 28; // Length of the message string, including the newline character
    ssize_t parent_bytes_written = rev_write(0, &parent_msg, parent_msg_len);
    rev_exit(99);
  }

  return 0;

}
