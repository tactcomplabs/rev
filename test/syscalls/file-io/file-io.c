#include "../../../common/syscalls/syscalls.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1024

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

int main() {
  char buffer[BUF_SIZE];

  const char path[12] = "test.txt";

  // Open the file "test.txt" under the current directory
  // rev_write(STDOUT_FILENO, OpenMsg, sizeof(OpenMsg));
  int fd = rev_openat(AT_FDCWD, path, 0, O_RDONLY);

  // Read from the file
  uint64_t bytesRead = rev_read(fd, buffer, 29);

  // Null-terminate the buffer so we can use it as a string
  buffer[bytesRead] = '\0';

  // Write to STDOUT
  rev_write(STDOUT_FILENO, buffer, bytesRead);
  // rev_write(STDOUT_FILENO, "\n", 1);
  // rev_write(STDOUT_FILENO, buffer, bytesRead);

  // Close the file and directory
  // rev_write(STDOUT_FILENO, CloseMsg, sizeof(CloseMsg));
  rev_close(fd);
  // rev_write(STDOUT_FILENO, ClosedMsg, sizeof(ClosedMsg));

  return 0;
}
