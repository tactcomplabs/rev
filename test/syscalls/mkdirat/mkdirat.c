// #include <unistd.h>
// #include <stdio.h>
// #include "sys/stat.h"

// int main() {
//   /* 
//    * NOTE: If you pass the directory string directly to `mkdir`
//    *       mkdir("/tmp") you will get garbage data.
//    *       This is a known issue with Text data in assembly 
//   */
//   int fd = -100;
//   int mode = 0777;
//   const char dir[8] = "/newdir";
//   int result = mkdirat(fd, &dir[0], mode);

//   if (result != 0) {
//     return 1;
//   } else {
//     return 0;
//   }
// }

#include <unistd.h>
#include <stdio.h>
#include "sys/stat.h"

int main() {
    int fd = -100;
    int mode = 0777;
    const char dir[13] = "/tmp/sst-new";
    int result = mkdirat(fd, dir, mode);

  if( result != 0){
    return 1;
  } else {
    return 0;
  }
}

