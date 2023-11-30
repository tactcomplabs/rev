#include "../../common/syscalls/syscalls.h"
#include <iostream>

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

int main() {
    // Using 'new' to allocate memory for an integer
    uint64_t* ptr = new uint64_t(5); // 'ptr' now points to an int with value 5

    // Accessing the value stored in the allocated memory
    assert(*ptr == 5);

    if( *ptr == 5 ) {
      rev_exit(5);
    }


    // Using 'delete' to deallocate the memory
    delete ptr;

    return 0;
}
