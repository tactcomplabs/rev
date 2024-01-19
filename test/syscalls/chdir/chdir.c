#include <unistd.h>
#include <stdio.h>
#include "../../../common/syscalls/syscalls.h"

int main() {
  /*
   * NOTE: If you pass the directory string directly to `chdir`
   *       chdir("/tmp") you will get garbage data.
   *       This is a known issue with Text data in assembly
  */
  const char dir[5] = "/tmp";
  int result = rev_chdir(&dir[0]);

  if (result != 0) {
    return 1;
  } else {
    return 0;
  }
}
