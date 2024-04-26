#include "fenv_test.h"

extern std::vector< void ( * )() > fenv_tests;
unsigned                           failures;

int main() {
  for( auto* test : fenv_tests )
    test();
  if( failures ) {
    std::cout << "\n" << failures << " tests failed" << std::endl;
    return 1;
  } else {
    std::cout << "\nAll tests passed" << std::endl;
    return 0;
  }
}
