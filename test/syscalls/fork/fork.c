#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    pid = _Fork();

    if (pid < 0) {
        _Exit(EXIT_FAILURE);
    }

    if (pid == 0) { // This is the child process
      _Exit(0);
      // const char child_msg[29] = "Hello from the child process";
      // ssize_t child_bytes_written = write(STDOUT_FILENO, &child_msg, sizeof(child_msg));
    _Exit(0);
    } else { // This is the parent process
    _Exit(0);
    }

    return 0;
}

