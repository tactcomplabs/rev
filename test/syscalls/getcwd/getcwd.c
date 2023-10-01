#include <unistd.h>
#include <stdio.h>
#include "../../../common/syscalls/syscalls.h"

int main() {
  /*
   * NOTE: If you pass the directory string directly to `chdir`
   *       chdir("/tmp") you will get garbage data.
   *       This is a known issue with Text data in assembly
  */
  char buf[100];
  int result = rev_getcwd(buf, 100);

  if (result != 0) {
    return 1;
  } else {
    return 0;
  }
}
