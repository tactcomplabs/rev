#include "../../../common/syscalls/syscalls.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

int main() {
    char buffer[BUF_SIZE];

    // Open the file "test.txt" under the current directory
    int fd = rev_openat(AT_FDCWD, "test.txt", O_RDONLY);
    if (fd == -1) {
        // handle error
    }

    // Read from the file
    uint64_t bytesRead = rev_read(fd, &buffer[0], 9);
    if (bytesRead == -1) {
      // Random exit code
      rev_exit(25);
    }

    // Null-terminate the buffer so we can use it as a string
    buffer[bytesRead] = '\0';

    // Write to STDOUT
    rev_write(STDOUT_FILENO, bytesRead, bytesRead);
    rev_write(STDOUT_FILENO, "\n", 1);
    rev_write(STDOUT_FILENO, &buffer[0], bytesRead);

    // Close the file and directory
    rev_close(fd);

    return 0;
}
