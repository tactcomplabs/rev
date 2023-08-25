#include <stdlib.h>
#include <stdio.h>

#include "../../../common/syscalls/syscalls.h"

#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }


int main() {
  int pid;
  uint64_t *addr;
  uint64_t *parent_addr;
  #define N 128

  pid = rev_fork();

  if (pid < 0) {
    rev_exit(pid);
  }

  if (pid == 0) { // This is the child process
    // const char msg[27] = "Greetings from the child!\n";// {72, 69, 0};
    // size_t msg_len = 27; // Length of the message string, including the newline character
    // ssize_t bytes_written = rev_write(0, &msg, msg_len);

    // Create an anonymous memory mapping
     addr = (uint64_t*)rev_mmap(0,                 // Let rev choose the address
                                N * sizeof(uint32_t),
                                PROT_READ | PROT_WRITE | PROT_EXEC, // RWX permissions
                                MAP_PRIVATE | MAP_ANONYMOUS, // Not shared, anonymous
                                -1,
                                0);

    for( uint32_t i=0; i<N; i++){
      addr[i] = i;
    }
    rev_exit(0);
  }
  else { 
    // This is the parent process
    // const char parent_msg[28] = "Greetings from the parent!\n";// {72, 69, 0};
    // size_t parent_msg_len = 28; // Length of the message string, including the newline character
    // ssize_t parent_bytes_written = rev_write(0, &parent_msg, parent_msg_len);

    // Create an anonymous memory mapping
     parent_addr = (uint64_t*)rev_mmap(0,                 // Let rev choose the address
                                N * sizeof(uint32_t),
                                PROT_READ | PROT_WRITE | PROT_EXEC, // RWX permissions
                                MAP_PRIVATE | MAP_ANONYMOUS, // Not shared, anonymous
                                -1,
                                0);

    for( uint32_t i=0; i<N; i++){
      parent_addr[i] = i;
    }
  }
  
  for(uint32_t i=0; i<N; i++){
    assert(addr[i] == i);
  }

  rev_write(0, "Child Data Good\n", 17);

  for(uint32_t i=0; i<N; i++){
    assert(parent_addr[i] == i);
  }
  rev_exit(99);

  return 0;

}

