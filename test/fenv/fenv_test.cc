#include "fenv_test.h"

#ifdef __riscv
#include "syscalls.h"
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif

extern void ( *fenv_tests[] )();
extern size_t num_fenv_tests;
unsigned      failures;

int main() {
  for( size_t i = 0; i < num_fenv_tests; ++i ) {
    fenv_tests[i]();

    // Make a useless syscall to have fencing effects
#ifdef __riscv
    rev_getuid();
#else
    syscall( SYS_getuid );
#endif
  }

  return !!failures;
}
