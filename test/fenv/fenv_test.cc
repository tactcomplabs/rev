#include "fenv_test.h"

#ifdef __riscv

#include "revalloc.h"
#include "syscalls.h"
extern std::vector<void ( * )(), Allocator<void ( * )()>> fenv_tests;

#else

#include <sys/syscall.h>
#include <unistd.h>
extern std::vector<void ( * )()> fenv_tests;

#endif

unsigned failures;

int main() {
  for( auto* test : fenv_tests ) {
    test();

    // Make a useless syscall to have fencing effects
#ifdef __riscv
    rev_getuid();
#else
    syscall( SYS_getuid );
#endif
  }

  if( failures ) {
    std::cout << "\n" << failures << " tests failed" << std::endl;
    return 1;
  } else {
    std::cout << "\nAll tests passed" << std::endl;
    return 0;
  }
}
