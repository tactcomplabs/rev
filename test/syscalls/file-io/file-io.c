#include "../../../common/syscalls/syscalls.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

int main() {
    char buffer[BUF_SIZE];

    const char OpenMsg[21] = "Opening ./test.txt\n\0";
    const char OpenedMsg[20] = "Opened ./test.txt\n\0";
    const char ReadMsg[26] = "Reading from ./test.txt\n\0";
    const char CloseMsg[21] = "Closing ./test.txt\n\0";
    const char ClosedMsg[20] = "Closed ./test.txt\n\0";

    // Open the file "test.txt" under the current directory
    rev_write(STDOUT_FILENO, &OpenMsg[0], sizeof(OpenMsg));
    int fd = rev_openat(AT_FDCWD, "test.txt", O_RDONLY);
    if (fd == -1) {
        // handle error
    }
    rev_write(STDOUT_FILENO, &OpenedMsg[0], sizeof(OpenedMsg));

  rev_write(STDOUT_FILENO, &ReadMsg[0], sizeof(ReadMsg));
    // Read from the file
    uint64_t bytesRead = rev_read(fd, &buffer[0], 28);
    if (bytesRead == -1) {
      // Random exit code
      rev_exit(25);
    }

    // Null-terminate the buffer so we can use it as a string
    buffer[bytesRead] = '\0';

    // Write to STDOUT
    // rev_write(STDOUT_FILENO, bytesRead, bytesRead);
    // rev_write(STDOUT_FILENO, "\n", 1);
    rev_write(STDOUT_FILENO, &buffer[0], bytesRead+1);

    // Close the file and directory
    rev_write(STDOUT_FILENO, &CloseMsg[0], sizeof(CloseMsg));
    rev_close(fd);
    rev_write(STDOUT_FILENO, &ClosedMsg[0], sizeof(ClosedMsg));

    return 0;
}
