#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // This is the child process
      const char child_msg[29] = "Hello from the child process";
      ssize_t child_bytes_written = write(STDOUT_FILENO, &child_msg, sizeof(child_msg));
    } else { // This is the parent process
      const char parent_msg[30] = "Hello from the parent process";
      ssize_t parent_bytes_written = write(STDOUT_FILENO, &parent_msg, sizeof(parent_msg));
        wait(NULL); // Wait for the child process to complete
    }

    return 0;
}

