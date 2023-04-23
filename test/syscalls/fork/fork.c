// #include <cstdint>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    unsigned pid;

    pid = _Fork();

    if (pid < 0) {
        _Exit(pid);
    }

    if (pid == 0) { // This is the child process
      const char msg[3] = "HE";// {72, 69, 0};
      size_t msg_len = 3; // Length of the message string, including the newline character
      ssize_t bytes_written = write(STDOUT_FILENO, &msg, msg_len);
      _Exit(0);
      // const char child_msg[29] = "Hello from the child process";
      // ssize_t child_bytes_written = write(STDOUT_FILENO, &child_msg, sizeof(child_msg));
    } else { // This is the parent process
      const char parent_msg[4] = "HIM";// {72, 69, 0};
      size_t parent_msg_len = 4; // Length of the message string, including the newline character
      ssize_t parent_bytes_written = write(STDOUT_FILENO, &parent_msg, parent_msg_len);
      _Exit(0);
    }

    return 0;
}

