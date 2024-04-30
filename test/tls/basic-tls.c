#include "../../common/syscalls/syscalls.h"

__thread int tls_var = 42;

int main() {
  rev_write( 0, &tls_var, sizeof( tls_var ) );
  return 0;
}
