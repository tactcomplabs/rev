#include <unistd.h>

int main() {
  // char msg[3];
  // msg[0] = 'T';
  // msg[1] = 'T';
  // msg[2] = '\0';

  // asm volatile("addi s2, zero, 72");
  // asm volatile("addi s3, zero, 69");
  // asm volatile("addi s4, zero, 0");

  const char msg[3] = "HE";// {72, 69, 0};
  size_t msg_len = 3; // Length of the message string, including the newline character

  ssize_t bytes_written = write(STDOUT_FILENO, &msg, msg_len);

  // if (1){// (bytes_written < 0) {
  //   return 1;
  // }

  return 0;
}
