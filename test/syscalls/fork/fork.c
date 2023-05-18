#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../../../common/syscalls/syscalls.h"

int main() {
    unsigned pid;

    pid = rev_fork();

    if (pid < 0) {
        rev_exit(pid);
    }

    if (pid == 0) { // This is the child process
      const char msg[27] = "Greetings from the child!\n";// {72, 69, 0};
      size_t msg_len = 27; // Length of the message string, including the newline character
      ssize_t bytes_written = rev_write(STDOUT_FILENO, &msg, msg_len);
      rev_exit(0);
      // const char child_msg[29] = "Hello from the child process";
      // ssize_t child_bytes_written = write(STDOUT_FILENO, &child_msg, sizeof(child_msg));
    } else { // This is the parent process
      const char parent_msg[28] = "Greetings from the parent!\n";// {72, 69, 0};
      size_t parent_msg_len = 28; // Length of the message string, including the newline character
      ssize_t parent_bytes_written = rev_write(STDOUT_FILENO, &parent_msg, parent_msg_len);
      rev_exit(420);
    }

    return 0;
}

