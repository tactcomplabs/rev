#include "fenv_test.h"
#include "revalloc.h"

extern std::vector<void ( * )(), Allocator<void ( * )()>> fenv_tests;
unsigned                                                  failures;

int main() {
  for( auto* test : fenv_tests ) {
    test();
    asm( " li a7, 81; ecall" );
  }
  if( failures ) {
    std::cout << "\n" << failures << " tests failed" << std::endl;
    return 1;
  } else {
    std::cout << "\nAll tests passed" << std::endl;
    return 0;
  }
}
